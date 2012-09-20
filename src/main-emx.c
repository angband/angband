/* File: main-emx.c */

/* Purpose: Support for OS/2 EMX Angband */

/* Author: ekraemer@pluto.camelot.de (Ekkehard Kraemer) */

#ifdef __EMX__

/*
 * === Instructions for using Angband 2.7.X with OS/2 ===
 *
 * The patches (file "main-emx.c") to compile Angband 2.7.X under OS/2
 * were written by ekraemer@pluto.camelot.de (Ekkehard Kraemer).
 *
 * TO COMPILE:
 *
 * - untar the archive into /angband (or /games/angband or whatever)
 * - change directory to /angband/src
 * - run "dmake -B -r -f makefile.emx" (not gmake or make)
 *
 * TO INSTALL:
 *
 * - change directory to /angband/src
 * - run "dmake -B -r -f makefile.emx install" (not gmake or make)
 * - copy your old savefile into ./lib/save and your old pref.prf into ./lib/user
 * - start /angband/angband.exe for one single window
 * - start /angband/startwnd.cmd for multiple windows
 *
 * TO REMOVE TEMPORARY FILES:
 *
 * - run 'dmake -B -r -f makefile.emx clean'
 *
 *
 * I use EMX 0.9b, but every EMX compiler since 0.8g or so should work
 * fine. EMX is available at ftp-os2.cdrom.com ("Hobbes"), as is dmake.
 *
 *  dmake:    ftp://ftp-os2.cdrom.com/all/program/dmake38X.zip
 *  EMX:      ftp://ftp-os2.cdrom.com/2_x/unix/emx???/   (most probably)
 *
 * Old savefiles must be renamed to follow the new "savefile" naming
 * conventions.  Either rename the savefile to "PLAYER", or start the
 * program with the "-uName" flag.  See "main.c" for details.  The
 * savefiles are stores in "./lib/save" if you forgot the old names...
 *
 *  Changes
 *  =======
 *
 *  When       By     Version   What
 *  -------------------------------------------------------------------
 *  
 *  18.11.95   EK      2.7.8    Added window/pipe code
 *                              Introduced __EMX__CLIENT__ hack 
 *
 *  15.12.95   EK      2.7.9    Updated for 2.7.9
 *                     beta     Added mirror view
 *                              Added number of line support in aclient
 *
 *  25.12.95   EK      2.7.9    Added 'install' target
 *                    non-beta  Updated installation hints
 *                              Uploaded binary to export.andrew.cmu.edu
 *
 *  25.01.96   EK      2.7.9    Updated for 2.7.9v3
 *                      v3      Removed (improved) keyboard hack
 *                              Introduced pref-emx.prf
 *                              Phew... Makefile.emx grows! (patches, export)
 *
 *  26.01.96   EK               Added files.uue target
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h> 
#include <sys/kbdscan.h>
#include <sys/video.h>
#include <io.h>
#include <os2.h>

#include "angband.h"

/*
 * termPipe* is sometimes cast to term* and vice versa, 
 * so "term t;" must be the first line 
 */

typedef struct
{
    term t;
    FILE *out;                  /* used by the ..._pipe_emx stuff */
} termPipe;

/*
 * Communication between server and client
 */

enum
{
    PIP_INIT,
    PIP_NUKE,
    PIP_XTRA,
    PIP_CURS,
    PIP_WIPE,
    PIP_TEXT,
};


/*
 * Prototypes!
 */
static errr Term_curs_emx(int x, int y);
static errr Term_wipe_emx(int x, int y, int n);
static errr Term_text_emx(int x, int y, int n, unsigned char a, cptr s);
static void Term_init_emx(term *t);
static void Term_nuke_emx(term *t);

/*
 * Current cursor "size"
 */
static int curs_start=0;
static int curs_end=0;

/*
 * Angband color conversion table
 *
 * Note that "Light Green"/"Yellow"/"Red" are used for
 * "good"/"fair"/"awful" stats and conditions.
 *
 * But "brown"/"light brown"/"orange" are really only used
 * for "objects" (such as doors and spellbooks), and these
 * values can in fact be re-defined.
 *
 * Future versions of Angband will probably allow the
 * "use" of more that 16 colors, so "extra" entries can be
 * made at the end of this table in preparation.
 */
static int colors[16]=
{
    F_BLACK,                    /* Black */
    F_WHITE|INTENSITY,          /* White */
    F_WHITE,                    /* XXX Gray */
    F_RED|INTENSITY,            /* Orange */
    F_RED,                      /* Red */
    F_GREEN,                    /* Green */
    F_BLUE,                     /* Blue */
    F_BROWN,                    /* Brown */
    F_BLACK|INTENSITY,          /* Dark-grey */
    F_WHITE,                    /* XXX Light gray */
    F_MAGENTA,                  /* Purple */
    F_YELLOW|INTENSITY,         /* Yellow */
    F_RED|INTENSITY,            /* Light Red */
    F_GREEN|INTENSITY,          /* Light Green */
    F_BLUE|INTENSITY,           /* Light Blue */
    F_BROWN|INTENSITY           /* Light brown */
};


/*
 * Display a cursor, on top of a given attr/char
 */
static errr Term_curs_emx(int x, int y)
{
    v_gotoxy(x,y);
    v_ctype(curs_start,curs_end);
    
    return (0);
}

/*
 * Erase a grid of space (as if spaces were printed)
 */
static errr Term_wipe_emx(int x, int y, int n)
{
    v_gotoxy(x,y);
    v_putn(' ',n);

    return (0);
}

/*
 * Draw some text, wiping behind it first
 */
static errr Term_text_emx(int x, int y, int n, unsigned char a, cptr s)
{
    /* Convert the color and put the text */
    v_attrib(colors[a & 0x0F]);
    v_gotoxy(x,y);
    v_putm(s,n);
    
    return (0);
}

/*
 * EMX initialization
 */
static void Term_init_emx(term *t) 
{
    v_init();
    v_getctype(&curs_start,&curs_end);
    v_clear();

    signal(SIGHUP,SIG_IGN);
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
/*  signal(SIGILL,SIG_IGN);  */
/*  signal(SIGTRAP,SIG_IGN); */
/*  signal(SIGABRT,SIG_IGN); */
/*  signal(SIGEMT,SIG_IGN);  */
/*  signal(SIGFPE,SIG_IGN);  */
/*  signal(SIGBUS,SIG_IGN);  */
/*  signal(SIGSEGV,SIG_IGN); */
/*  signal(SIGSYS,SIG_IGN);  */
    signal(SIGPIPE,SIG_IGN);
    signal(SIGALRM,SIG_IGN);
/*  signal(SIGTERM,SIG_IGN); */
    signal(SIGUSR1,SIG_IGN);
    signal(SIGUSR2,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGBREAK,SIG_IGN);
}

/*
 * EMX shutdown
 */
static void Term_nuke_emx(term *t)
{
    /* Move the cursor to bottom of screen */
    v_gotoxy(0,23);
        
    /* Restore the cursor (not necessary) */
    v_ctype(curs_start,curs_end);

    /* Set attribute to gray on black */
    v_attrib(F_WHITE);

    /* Clear the screen */
    v_clear();
}


#ifndef __EMX__CLIENT__           

/*
 * Oh no, more prototypes!
 */
static errr CheckEvents(int returnImmediately);
static errr Term_xtra_pipe_emx(int n, int v);
static errr Term_curs_pipe_emx(int x, int y);
static errr Term_wipe_pipe_emx(int x, int y, int n);
static errr Term_text_pipe_emx(int x, int y, int n, unsigned char a, cptr s);
static void Term_init_pipe_emx(term *t);
static void Term_nuke_pipe_emx(term *t);
static FILE *initPipe(char *name);
static void initPipeTerm(termPipe *pipe,char *name,term **term);

/*
 * Main initialization function
 */
errr init_emx(void);

/*
 * The screens 
 */
static termPipe term_screen_body,
                term_screen_recall,
                term_screen_choice,
                term_screen_mirror;

/*
 * Check for events -- called by "Term_scan_emx()"
 *
 * Note -- this is probably NOT the most efficient way
 * to "wait" for a keypress (TERM_XTRA_EVENT).
 *
 * _read_kbd(0,0,0) does this:
 *       - if no key is available, return -1
 *       - if extended key available, return 0
 *       - normal key available, return ASCII value (1 ... 255)
 *
 *  _read_kbd(0,1,0) waits on a key and then returns
 *       - 0, if it's an extended key
 *       - the ASCII value, if it's a normal key
 *
 *  *If* _read_kbd() returns 0, *then*, and only then, the next
 *  call to _read_kbd() will return the extended scan code.
 *
 * See "main-ibm.c" for a "better" use of "macro sequences".
 *
 * Note that this file does *NOT* currently extract modifiers
 * (such as Control and Shift).  See "main-ibm.c" for a method.
 *
 * XXX XXX XXX XXX The "key handling" really needs to be fixed.
 * See "main-ibm.c" for more information about "macro encoding".
 */
static errr CheckEvents(int returnImmediately)
{
    int k = 0, ke = 0;

    /* Get key */
    k=_read_kbd(0,returnImmediately?0:1,0);

    /* Nothing ready */
    if (k < 0) return (1);

    /* Get an extended scan code */
    if (!k) ke = _read_kbd(0,1,0);

    /* Normal keypresses */
    if (k)
    {
        /* Enqueue the key */
        Term_keypress(k);

        /* Success */
        return (0);
    }

    /* Hack -- introduce a macro sequence */
    Term_keypress(31);

    /* XXX We're not able to extract shift/ctrl/alt key information here. */

    /* Hack -- send the key sequence */
    Term_keypress('0' + (ke % 1000) / 100);
    Term_keypress('0' + (ke % 100) / 10);
    Term_keypress('0' + (ke % 10));

    /* Hack --  end the macro sequence */
    Term_keypress(13);

    /* Success */
    return (0);
}

/*
 * Do a special thing (beep, flush, etc)
 */
static errr Term_xtra_emx(int n, int v)
{
    switch (n)
    {

        case TERM_XTRA_INVIS: 
            v_hidecursor(); 
            return (0);

        case TERM_XTRA_BEVIS: 
            v_ctype(curs_start,curs_end); 
            return (0);

        case TERM_XTRA_NOISE: 
            putchar(7); 
            return (0);

        case TERM_XTRA_FLUSH:
            while (!CheckEvents(TRUE));
            return 0;

        case TERM_XTRA_EVENT:

            /* Process an event */
            return (CheckEvents(!v));

            /* Success */
            return (0);

        case TERM_XTRA_CLEAR:
            v_clear();
            return (0);
    }

    return (1);
}

static errr Term_xtra_pipe_emx(int n, int v)
{
    termPipe *tp=(termPipe*)Term;

    switch (n)
    {
        case TERM_XTRA_NOISE: 
            putchar(7); 
            return (0);

        case TERM_XTRA_INVIS: 
            return (0);

        case TERM_XTRA_BEVIS: 
            return (0);

        case TERM_XTRA_EVENT: 
            return (CheckEvents(FALSE));

        case TERM_XTRA_CLEAR:

            if (!tp->out) return -1;
    
            fputc(PIP_XTRA,tp->out);
            fwrite(&n,sizeof(n),1,tp->out);
            fwrite(&v,sizeof(v),1,tp->out);
            fflush(tp->out);

            return (0);
    }

    return (1);
}



static errr Term_curs_pipe_emx(int x, int y)
{
    termPipe *tp=(termPipe*)Term;

    if (!tp->out) return -1;
    
    fputc(PIP_CURS,tp->out);
    fwrite(&x,sizeof(x),1,tp->out);
    fwrite(&y,sizeof(y),1,tp->out);
    fflush(tp->out);

    return (0);
}


static errr Term_wipe_pipe_emx(int x, int y, int n)
{
    termPipe *tp=(termPipe*)Term;

    if (!tp->out) return -1;
    
    fputc(PIP_WIPE,tp->out);
    fwrite(&x,sizeof(x),1,tp->out);
    fwrite(&y,sizeof(y),1,tp->out);
    fwrite(&n,sizeof(n),1,tp->out);
    fflush(tp->out);

    return (0);
}


static errr Term_text_pipe_emx(int x, int y, int n, unsigned char a, cptr s)
{
    termPipe *tp=(termPipe*)Term;

    if (!tp->out) return -1;
    
    fputc(PIP_TEXT,tp->out);
    fwrite(&x,sizeof(x),1,tp->out);
    fwrite(&y,sizeof(y),1,tp->out);
    fwrite(&n,sizeof(n),1,tp->out);
    fwrite(&a,sizeof(a),1,tp->out);
    fwrite(s,n,1,tp->out);
    fflush(tp->out);

    return (0);
}


static void Term_init_pipe_emx(term *t)
{
    termPipe *tp=(termPipe*)t;

    if (tp->out)
    {
        fputc(PIP_INIT,tp->out);
        fflush(tp->out);
    }
}


static void Term_nuke_pipe_emx(term *t)
{
    termPipe *tp=(termPipe*)t;

    if (tp->out)
    {
        fputc(PIP_NUKE,tp->out); /* Terminate client */
        fflush(tp->out);
        fclose(tp->out);         /* Close Pipe */
        tp->out=NULL;            /* Paranoia */
    }
}

static void initPipeTerm(termPipe *pipe,char *name,term **termTarget)
{
    term *t;

    t=(term*)pipe;

    if ((pipe->out=initPipe(name))!=NULL)
    {
        /* Initialize the term */
        term_init(t, 80, 24, 1);  

        /* Special hooks */
        t->init_hook = Term_init_pipe_emx;
        t->nuke_hook = Term_nuke_pipe_emx;

        /* Add the hooks */
        t->text_hook = Term_text_pipe_emx;
        t->wipe_hook = Term_wipe_pipe_emx;
        t->curs_hook = Term_curs_pipe_emx;
        t->xtra_hook = Term_xtra_pipe_emx;
        
        /* Save it */
        *termTarget = t;

        /* Activate it */
        Term_activate(t); 
    }
}

/*
 * Prepare "term.c" to use "__EMX__" built-in video library
 */
errr init_emx(void)
{
    term *t;

    /* Initialize the pipe windows */
    initPipeTerm(&term_screen_recall,"recall",&term_recall);
    initPipeTerm(&term_screen_choice,"choice",&term_choice);
    initPipeTerm(&term_screen_mirror,"mirror",&term_mirror);

    /* Initialize main window */
    t = (term*)(&term_screen_body);
    
    /* Initialize the term -- big key buffer */
    term_init(t, 80, 24, 1024);    

    /* Special hooks */
    t->init_hook = Term_init_emx;
    t->nuke_hook = Term_nuke_emx;

    /* Add the hooks */
    t->text_hook = Term_text_emx;
    t->wipe_hook = Term_wipe_emx;
    t->curs_hook = Term_curs_emx;
    t->xtra_hook = Term_xtra_emx;
    
    /* Save it */
    term_screen = t;

    /* Activate it */
    Term_activate(t);

    /* Success */
    return (0);
}

static FILE *initPipe(char *name)
{
    char buf[256];
    FILE *fi;

    sprintf(buf,"\\pipe\\angband\\%s",name);   /* Name of pipe */
    fi=fopen(buf,"wb");                        /* Look for server */
    return fi;
}

#else /* __EMX__CLIENT__ */

int main(int argc, char **argv)
{
    int c, end = 0, lines = 25;
    int x, y, h, n, v;

    FILE *in=NULL;
    char a;
    char buf[160];
    HPIPE pipe;
    APIRET rc;

    /* Check command line */
    if (argc!=2 && argc!=3)
    {
        printf("Usage: %s choice|recall|mirror [number of lines]\n"
               "Start this before angband.exe\n",argv[0]);
        exit(1);
    }

    if (argc==3) lines = atoi(argv[2]);
    if (lines <= 0) lines = 25;

    printf("Looking for Angband... press ^C to abort\n");

    sprintf(buf,"\\pipe\\angband\\%s",argv[1]);

    do
    {
        rc=DosCreateNPipe((PSZ)buf,          /* Create pipe */
                          &pipe,
                          NP_ACCESS_INBOUND,
                          NP_WAIT|NP_TYPE_BYTE|NP_READMODE_BYTE|1,
                          1,                 /* No output buffer */
                          1,                 /* No input buffer */
                          -1);

        if (rc)                              /* Pipe not created */
        {
            printf("DosCreateNPipe: rc=%ld, pipe=%ld\n",(long)rc,(long)pipe);
            break;
        }

        do                                         
        {
            rc=DosConnectNPipe(pipe);        /* Wait for angband to connect */
            if (!rc) break;
            _sleep2(500);                    /* Sleep for 0.5s  */
        } while (_read_kbd(0,0,0)==-1);      /* Until key pressed */

        if (rc) break;

        h=_imphandle(pipe);                  /* Register handle with io */
        setmode(h,O_BINARY);                 /* Make it binary */
        in=fdopen(h,"rb");                   /* Register handle with stdio */

    } while (0);           /* We don't need no stinking exception handling <g> */

    if (!in)
    {
        printf("Sorry, the pipe connection to Angband could not be established.\n");
        exit(1);
    }

    printf("Connected.\n");

    sprintf(buf,"mode co80,%d",lines);
    system(buf);

    /* Infinite loop */
    while (!end) {

        /* Get command */
        c = fgetc(in);

        switch (c)
        {
            case PIP_XTRA:
                if (!fread(&n,sizeof(x),1,in) ||
                    !fread(&v,sizeof(y),1,in))
                    abort();

                /* This hack prevents another hack */
                switch (n)
                {
                    case TERM_XTRA_CLEAR: 
                        v_clear(); 
                        break;

                    default:
                        printf("Sorry, angband.exe and aclient.exe don't fit together.\n");
                        exit(1);
                }
                
                break;

            case PIP_CURS:
                if (!fread(&x,sizeof(x),1,in) ||
                    !fread(&y,sizeof(y),1,in))
                    abort();
                Term_curs_emx(x,y);
                break;                
                
            case PIP_WIPE:
                if (!fread(&x,sizeof(x),1,in) ||
                    !fread(&y,sizeof(y),1,in) ||
                    !fread(&n,sizeof(n),1,in))
                    abort();
                Term_wipe_emx(x,y,n);
                break;                
                
            case PIP_TEXT:
                if (!fread(&x,sizeof(x),1,in) ||
                    !fread(&y,sizeof(y),1,in) ||
                    !fread(&n,sizeof(n),1,in) ||
                    !fread(&a,sizeof(a),1,in) || (n > 160) ||
                    !fread(buf,n,1,in))
                    abort();
                Term_text_emx(x,y,n,a,buf);
                break;

            case PIP_INIT:
                Term_init_emx(NULL);
                break;
            
            case PIP_NUKE:
            case EOF:
            default:
                Term_nuke_emx(NULL);
                end=1;
                break;
        }                
    }

    return 0;
}

#endif /* __EMX__CLIENT__ */

#endif /* __EMX__ */

/*
 * Local Variables:
 * comment-column: 45
 * End:
 *
 */
