/* File: main-cur.c */

/* Purpose: Actual Unix Curses support for Angband */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

/*
 * Some annoying machines define "bool" in various packages
 * Note that this "redefinition" should work on any machine.
 */

#define bool bool_hack
#include "angband.h"
#undef bool


#ifdef USE_CUR


#ifndef __MAKEDEPEND__


# ifdef linux
#  include <bsd/sgtty.h>
# endif

# ifdef GEMDOS
#  define ATARIST_MWC
#  include "curses.h"
#  include <osbind.h>
   char               *getenv();
# endif


# ifdef MSDOS

/*** OPEN MSDOS ***/
#  include <process.h>
#  if defined(ANSI)
#   include "ms_ansi.h"
#  endif
/*** SHUT MSDOS ***/

# else /* MSDOS */
#  ifndef ATARIST_MWC

/*** OPEN NORMAL ***/
#   include <signal.h>
#   undef TRUE
#   undef FALSE
#   include <curses.h>
#   ifndef VMS
#    include <sys/ioctl.h>
#   endif
#   ifdef USG
#    ifndef __MINT__
#     include <termio.h>
#    endif
#   endif

#ifdef M_XENIX
# include <sys/select.h>
#endif

#ifdef USG
# ifndef __MINT__
#  include <termio.h>
# endif
#else
# if defined(atarist) && defined(__GNUC__) && !defined(__MINT__)
   /* doesn't have <sys/wait.h> */
# else
#  include <sys/wait.h>
# endif
# include <sys/resource.h>
# include <sys/param.h>
#  include <sys/param.h>
#  include <sys/file.h>
#  include <sys/types.h>
#  ifndef VMS
#   include <sys/wait.h>
#  endif /* !VMS */

#endif

/* Hack --  Brute force never hurt... [cjh] */
# if defined(__MINT__) && !defined(_WAIT_H)
#  include <wait.h>
# endif


/*** SHUT NORMAL ***/

#  endif
# endif /* MSDOS */

#endif /* __MAKEDEPEND__ */



extern char *getenv();

#ifdef ATARIST_MWC
extern WINDOW *newwin();
#endif


#if !defined(MSDOS) && !defined(ATARIST_MWC) && !defined(__MINT__)
#ifdef USG
static struct termio save_termio;
#else
#ifndef VMS
static struct ltchars save_special_chars;
static struct sgttyb save_ttyb;
static struct tchars save_tchars;
static int          save_local_chars;
#endif
#endif
#endif


static int     curses_on = FALSE;


/*
 * The main screen
 */
static term term_screen_body;



/*
 * Shut down curses (restore_term)
 */
static void unix_restore_curses()
{
    if (!curses_on) return;

    (void)refresh();

    /* this moves curses to bottom right corner */
    mvcur(curscr->_cury, curscr->_curx, LINES - 1, 0);

    endwin();			   /* exit curses */

    (void)fflush(stdout);

/* restore the saved values of the special chars */
#ifdef USG
# if !defined(MSDOS) && !defined(ATARIST_MWC) && !defined(__MINT__)
    (void)ioctl(0, TCSETA, (char *)&save_termio);
# endif
#else
# ifndef VMS
    (void)ioctl(0, TIOCSLTC, (char *)&save_special_chars);
    (void)ioctl(0, TIOCSETP, (char *)&save_ttyb);
    (void)ioctl(0, TIOCSETC, (char *)&save_tchars);
    (void)ioctl(0, TIOCLSET, (char *)&save_local_chars);
# endif
#endif

    curses_on = FALSE;
}



/*
 * Hack -- Suspend Curses
 * See "signals.h" for usage
 */
static void unix_suspend_curses(int go)
{

#ifdef USG

/*
 * for USG systems with BSDisms that have SIGTSTP defined,
 * but don't actually implement it.  XXX What?
 */

#else

    static struct sgttyb  tbuf;
    static struct ltchars lcbuf;
    static struct tchars  cbuf;
    static int            lbuf;
    static long           time();

    /* Step One */
    if (go)
    {
        (void)ioctl(0, TIOCGETP, (char *)&tbuf);
        (void)ioctl(0, TIOCGETC, (char *)&cbuf);
        (void)ioctl(0, TIOCGLTC, (char *)&lcbuf);
        (void)ioctl(0, TIOCLGET, (char *)&lbuf);

        unix_restore_curses();
    }

    /* Step 2 */
    else
    {
        curses_on = TRUE;

        (void)ioctl(0, TIOCSETP, (char *)&tbuf);
        (void)ioctl(0, TIOCSETC, (char *)&cbuf);
        (void)ioctl(0, TIOCSLTC, (char *)&lcbuf);
        (void)ioctl(0, TIOCLSET, (char *)&lbuf);

        (void)touchwin(curscr);
        (void)wrefresh(curscr);

        cbreak();
        noecho();
    }

#endif

}


/*
 * Set up the terminal for a standard "moria" type game
 */
static void moriaterm()
{

#if !defined(MSDOS) && !defined(ATARIST_MWC) && !defined(__MINT__)
#ifdef USG

    struct termio  tbuf;

#else

    struct ltchars lbuf;
    struct tchars  buf;

#endif
#endif

    curses_on = TRUE;

#ifndef BSD4_3
    crmode();
#else
    cbreak();
#endif

    noecho();

    /* can not use nonl(), because some curses do not handle it correctly */

#ifdef MSDOS
    msdos_raw();
#else

#if !defined(ATARIST_MWC) && !defined(__MINT__)
#ifdef USG

    /* disable all of the normal special control characters */
    (void)ioctl(0, TCGETA, (char *)&tbuf);
    tbuf.c_cc[VINTR] = (char)3;	   /* control-C */
    tbuf.c_cc[VQUIT] = (char)-1;
    tbuf.c_cc[VERASE] = (char)-1;
    tbuf.c_cc[VKILL] = (char)-1;
    tbuf.c_cc[VEOF] = (char)-1;
    tbuf.c_cc[VEOL] = (char)-1;
    tbuf.c_cc[VEOL2] = (char)-1;
    tbuf.c_cc[VMIN] = 1;	   /* Input should wait for at least 1 char */
    tbuf.c_cc[VTIME] = 0;	   /* no matter how long that takes. */
    (void)ioctl(0, TCSETA, (char *)&tbuf);

#else
#ifndef VMS

/*
 * disable all of the special characters except the suspend char, interrupt
 * char, and the control flow start/stop characters
 */

    (void)ioctl(0, TIOCGLTC, (char *)&lbuf);
    lbuf.t_suspc = (char)26;	   /* control-Z */
    lbuf.t_dsuspc = (char)-1;
    lbuf.t_rprntc = (char)-1;
    lbuf.t_flushc = (char)-1;
    lbuf.t_werasc = (char)-1;
    lbuf.t_lnextc = (char)-1;
    (void)ioctl(0, TIOCSLTC, (char *)&lbuf);

    (void)ioctl(0, TIOCGETC, (char *)&buf);
    buf.t_intrc = (char)3;	   /* control-C */
    buf.t_quitc = (char)-1;
    buf.t_startc = (char)17;	   /* control-Q */
    buf.t_stopc = (char)19;	   /* control-S */
    buf.t_eofc = (char)-1;
    buf.t_brkc = (char)-1;
    (void)ioctl(0, TIOCSETC, (char *)&buf);

#endif
#endif
#endif
#endif

}




/*
 * Nuke the "curses" system
 */
static void Term_nuke_cur(term *t)
{
    /* XXX Restore the terminal */
    unix_restore_curses();
}





/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_cur(int x, int y, int z)
{
    /* Literally move the cursor */
    move(y,x);

    return (0);
}

/*
 * Erase a grid of space
 * Hack -- try to be "semi-efficient".
 */
static errr Term_wipe_cur(int x, int y, int w, int h)
{
    int dx, dy;

    if (!x && !y && (w >= 80) && (h >= 24))
    {
        touchwin(stdscr);
        (void)clear();
    }

    else if (!x && (h >= 24) && (w >= 80))
    {
        move(y,x);
        clrtobot();
    }

    else if (w >= 80)
    {
        for (dy = 0; dy < h; ++dy)
        {
            move(y+dy,x);
            clrtoeol();
        }
    }

    else
    {
        for (dy = 0; dy < h; ++dy)
        {
            move(y+dy,x);
            for (dx = 0; dx < w; ++dx) addch(' ');
        }
    }

    /* Hack -- Fix the cursor */
    move(y,x);

    return (0);
}


/*
 * Place some text on the screen using an attribute
 * Unfortunately, the "attribute" is ignored...
 */
static errr Term_text_cur(int x, int y, int n, byte a, cptr s)
{
    int i;
    char buf[81];

    /* Hack -- force "termination" of the text */
    if (n > 80) n = 80;
    for (i = 0; (i < n) && s[i]; ++i) buf[i] = s[i];
    buf[n]=0;

    /* Move the cursor and dump the string */
    move(y, x);
    addstr(buf);

    return (0);
}



/*
 * Provides for a timeout on input. Does a non-blocking read, consuming the
 * data if any, and then returns 1 if data was read, zero otherwise.
 *
 * Porting:
 *
 * In systems without the select call, but with a sleep for fractional numbers
 * of seconds, one could sleep for the time and then check for input.
 *
 * In systems which can only sleep for whole number of seconds, you might sleep
 * by writing a lot of nulls to the terminal, and waiting for them to drain,
 * or you might hack a static accumulation of times to wait. When the
 * accumulation reaches a certain point, sleep for a second. There would need
 * to be a way of resetting the count, with a call made for commands like run
 * or rest.
 *
 * Currently, miscrosec is zero, but we could use this in some kbhit()'s.
 *
 * Note: Was using: defined(BSD4_3) || defined(M_XENIX) || defined(linux)
 * to test explicitly for the presense of the "FD_SET" macro.
 */
static int check_input(int microsec)
{
    int result = 0;

#if defined(USG) && !defined(M_XENIX)
    int                 arg;
#else
    struct timeval      tbuf;

#ifdef FD_SET
    fd_set              smask;
    fd_set		*no_fds = NULL;
#else
    int                 smask;
    int			*no_fds = NULL;
#endif

#endif


#if defined(USG) && !defined(M_XENIX)

    /*** SysV code (?) ***/

    /* XXX Hack -- mod 128, sleep one sec every 128 turns */
    if (microsec != 0 && (turn & 0x7F) == 0) (void)sleep(1);

    /* XXX Hack -- Can't check for input, but can do non-blocking read */
    arg = 0;
    arg = fcntl(0, F_GETFL, arg);
    arg |= O_NDELAY;
    (void)fcntl(0, F_SETFL, arg);

    result = getchar();

    arg = 0;
    arg = fcntl(0, F_GETFL, arg);
    arg &= ~O_NDELAY;
    (void)fcntl(0, F_SETFL, arg);

    if (result == EOF) return 0;

#else

    /*** Do a nice clean "select" ***/

    tbuf.tv_sec = 0;
    tbuf.tv_usec = microsec;

#ifdef FD_SET
    FD_ZERO(&smask);
    FD_SET(0, &smask);		/* standard input is bit 0 */
#else
    smask = 0x0001;		/* standard input is bit 0 */
#endif

    /* If we time out, no key ready */
    if (select(1, &smask, no_fds, no_fds, &tbuf) != 1) return (0);

    /* Get a key */
    result = getchar();

    /* See "EOF" handling below */
    if (result == EOF) exit_game_panic();

#endif

    /* There is a key ready, return it */
    return (result);
}




/*
 * Check for events
 */
static errr Term_xtra_cur_check(int v)
{
    int i;

    /* Get a keypress */
    i = check_input(0);

    /* Nothing ready */
    if (!i) return (1);

    /* Enqueue the keypress */
    Term_keypress(i);

    /* Success */
    return (0);
}


/*
 * Wait for a keypress.
 */
static errr Term_xtra_cur_event(int v)
{
    int i;

    /* Get a keypress */
    i = getchar();

    /* Broken input is special */
    if (i == EOF) exit_game_panic();

    /* Enqueue the keypress */
    Term_keypress(i);

    return (0);
}


/*
 * Suspend/Resume
 */
static errr Term_xtra_cur_level(int v)
{
    switch (v)
    {
        case TERM_LEVEL_HARD_SHUT: unix_suspend_curses(1); break;

        /* Come back from suspend */
        case TERM_LEVEL_HARD_OPEN: unix_suspend_curses(0); break;
    }

    return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_cur(int n, int v)
{
    /* Analyze the request */
    switch (n)
    {
        /* Make a noise */
        case TERM_XTRA_NOISE: (void)write(1, "\007", 1); return (0);

        /* Flush the Curses buffer */
        case TERM_XTRA_FLUSH: (void)refresh(); return (0);

#ifdef SYS_V

        /* XXX Make the cursor invisible */
        case TERM_XTRA_INVIS: curs_set(0); return (0);

        /* XXX Make the cursor visible */
        case TERM_XTRA_BEVIS: curs_set(1); return (0);

#endif

        /* Suspend/Resume curses */
        case TERM_XTRA_LEVEL: return (Term_xtra_cur_level(v));

        /* Check for event */
        case TERM_XTRA_CHECK: return (Term_xtra_cur_check(v));

        /* Wait for event */
        case TERM_XTRA_EVENT: return (Term_xtra_cur_event(v));
    }

    return (1);
}


/*
 * Prepare "curses" for use by the file "term.c"
 * Installs the "hook" functions defined above
 */
errr init_cur(void)
{
    int i, y, x, err;

    term *t = &term_screen_body;


#if defined(VMS) || defined(MSDOS) || \
    defined(ATARIST_MWC) || defined(__MINT__)

    /* Nothing */

#else
#ifdef USG

    (void)ioctl(0, TCGETA, (char *)&save_termio);

#else

    (void)ioctl(0, TIOCGLTC, (char *)&save_special_chars);
    (void)ioctl(0, TIOCGETP, (char *)&save_ttyb);
    (void)ioctl(0, TIOCGETC, (char *)&save_tchars);
    (void)ioctl(0, TIOCLGET, (char *)&save_local_chars);

#endif
#endif


#ifdef ATARIST_MWC
    initscr();
    err = (ERR)
#else
#if defined(USG) && !defined(PC_CURSES)	/* PC curses returns ERR */
    err = (initscr() == NULL);
#else
    err = (initscr() == ERR);
#endif
#endif

    /* Quit on error */
    if (err) quit("failure initializing curses");

    /* Check we have enough screen. -CJS- */
    err = (LINES < 24 || COLS < 80);

    /* Quit with message */
    if (err) quit("screen too small (need at least 80x24)");

#ifdef SIGTSTP
#ifdef __MINT__
    (void)signal(SIGTSTP, (__Sigfunc)unix_suspend_curses);
#else
    (void)signal(SIGTSTP, unix_suspend_curses);
#endif
#endif

    (void)clear();
    (void)refresh();

    moriaterm();


    /*** Check tab settings ***/

#ifdef ATARIST_MWC
    move(0, 0);
#else
    (void)move(0, 0);
#endif

    for (i = 1; i < 10; i++)
    {

#ifdef ATARIST_MWC
        addch('\t');
#else
        (void)addch('\t');
#endif

        getyx(stdscr, y, x);
        if (y != 0 || x != i * 8) break;
    }

    /* Verify tab stops */
    if (i != 10) quit("must have 8-space tab-stops");


    /* Initialize the term */
    term_init(t, 80, 24, 64);

    /* Hack -- shutdown hook */
    t->nuke_hook = Term_nuke_cur;

    /* Stick in some hooks */
    t->text_hook = Term_text_cur;
    t->wipe_hook = Term_wipe_cur;
    t->curs_hook = Term_curs_cur;
    t->xtra_hook = Term_xtra_cur;

    /* Save the term */
    term_screen = t;

    /* Activate it */
    Term_activate(term_screen);


    /* Success */
    return (0);
}







#if 0

/*
 * Another unused function -- note the "curses" dependancies
 */
void shell_out()
{
#ifndef MACINTOSH
#ifdef USG
#if !defined(MSDOS) && !defined(ATARIST_MWC) && !defined(__MINT__)
    struct termio       tbuf;

#endif
#else
    struct sgttyb       tbuf;
    struct ltchars      lcbuf;
    struct tchars       cbuf;
    int                 lbuf;

#endif
#ifdef MSDOS
    char               *comspec, key;

#else
#ifdef ATARIST_MWC
    char                comstr[80];
    char               *str;
    extern char       **environ;

#else
    int                 val;
    char               *str;

#endif
#endif

    save_screen();
/* clear screen and print 'exit' message */
    clear_screen();

#ifndef ATARIST_MWC
    put_str("[Entering shell, type 'exit' to resume your game.]\n", 0, 0);
#else
    put_str("[Escaping to shell]\n", 0, 0);
#endif
    Term_fresh();

#ifdef USG
#if !defined(MSDOS) && !defined(ATARIST_MWC) && !defined(__MINT__)
    (void)ioctl(0, TCGETA, (char *)&tbuf);
#endif
#else
#ifndef VMS
    (void)ioctl(0, TIOCGETP, (char *)&tbuf);
    (void)ioctl(0, TIOCGETC, (char *)&cbuf);
    (void)ioctl(0, TIOCGLTC, (char *)&lcbuf);
    (void)ioctl(0, TIOCLGET, (char *)&lbuf);
#endif
#endif

/* would call nl() here if could use nl()/nonl(), see moriaterm() */

#ifndef BSD4_3
    nocrmode();
#else
    nocbreak();
#endif

#ifdef MSDOS
    msdos_noraw();
#endif

    echo();
    ignore_signals();

#ifdef MSDOS			   /* { */
    if ((comspec = getenv("COMSPEC")) == NULL
        || spawnl(P_WAIT, comspec, comspec, C_NULL) < 0) {
        clear_screen();		   /* BOSS key if shell failed */
        put_str("M:\\> ", 0, 0);
        do {
            key = inkey();
        } while (key != '!');
    }
#else				   /* MSDOS }{ */
#ifndef ATARIST_MWC
    val = fork();
    if (val == 0) {
#endif
        default_signals();
#ifdef USG
#if !defined(MSDOS) && !defined(ATARIST_MWC) && !defined(__MINT__)
        (void)ioctl(0, TCSETA, (char *)&save_termio);
#endif
#else
#ifndef VMS
        (void)ioctl(0, TIOCSLTC, (char *)&save_special_chars);
        (void)ioctl(0, TIOCSETP, (char *)&save_ttyb);
        (void)ioctl(0, TIOCSETC, (char *)&save_tchars);
        (void)ioctl(0, TIOCLSET, (char *)&save_local_chars);
#endif
#endif

        if ((str = getenv("SHELL")))
#ifndef ATARIST_MWC
            (void)execl(str, str, C_NULL);
#else
            system(str);
#endif
        else
#ifndef ATARIST_MWC
            (void)execl("/bin/sh", "sh", C_NULL);
#endif
        msg_print("Cannot execute shell.");
#ifndef ATARIST_MWC

        /* Actually abort everything */
        quit(NULL);
    }
    if (val == -1) {
        msg_print("Fork failed. Try again.");
        return;
    }
#ifdef USG
    (void)wait((int *)(NULL));
#else
    (void)wait((union wait *)(NULL));
#endif
#endif				   /* ATARIST_MWC */
#endif				   /* MSDOS } */

    restore_signals();
/* restore the cave to the screen */
    restore_screen();

#ifndef BSD4_3
    crmode();
#else
    cbreak();
#endif

    noecho();

/* would call nonl() here if could use nl()/nonl(), see moriaterm() */
#ifdef MSDOS
    msdos_raw();
#endif
/* disable all of the local special characters except the suspend char */
/* have to disable ^Y for tunneling */
#ifdef USG
#if !defined(MSDOS) && !defined(ATARIST_MWC) && !defined(__MINT__)
    (void)ioctl(0, TCSETA, (char *)&tbuf);
#endif
#else
#ifndef VMS
    (void)ioctl(0, TIOCSLTC, (char *)&lcbuf);
    (void)ioctl(0, TIOCSETP, (char *)&tbuf);
    (void)ioctl(0, TIOCSETC, (char *)&cbuf);
    (void)ioctl(0, TIOCLSET, (char *)&lbuf);
#endif
#endif
    (void)wrefresh(curscr);
#endif
}


#if 0

/*
 * This is not used.  It will only work on some systems.
 */

/*
 * A command for the operating system. Standard library function 'system' is
 * unsafe, as it leaves various file descriptors open. This also is very
 * careful with signals and interrupts, and does rudimentary job control, and
 * puts the terminal back in a standard mode.
 */
int system_cmd(cptr p)
{
    int                 pgrp, pid, i, mask;
    union wait          w;
    extern char        *getenv();

    mask = sigsetmask(~0);	   /* No interrupts. */
    restore_term();		   /* Terminal in original state. */

    /* Are we in the control terminal group? */
    if (ioctl(0, TIOCGPGRP, (char *)&pgrp) < 0 || pgrp != getpgrp(0)) {
        pgrp = (-1);
    }

    pid = fork();
    if (pid < 0) {
        (void)sigsetmask(mask);

    xxx xxx xxx
    /* No longer defined -- see "term.c" */
    moriaterm();		   /* Terminal in moria mode. */
    xxx xxx xxx

        return (-1);
    }
    if (pid == 0) {
        (void)sigsetmask(0);	   /* Interrupts on. */
    /* Transfer control terminal. */
        if (pgrp >= 0) {
            i = getpid();
            (void)ioctl(0, TIOCSPGRP, (char *)&i);
            (void)setpgrp(i, i);
        }
        for (i = 2; i < 30; i++)
            (void)close(i);	   /* Close all but standard in and out. */
        (void)dup2(1, 2);	   /* Make standard error as standard out. */
        if (p == 0 || *p == 0) {
            p = getenv("SHELL");
            if (p)
                execl(p, p, 0);
            execl("/bin/sh", "sh", 0);
        }
        else {
            execl("/bin/sh", "sh", "-c", p, 0);
        }

        /* Hack (?) */
        _exit(1);
    }

/* Wait for child termination. */
    for (;;) {
        i = wait3(&w, WUNTRACED, (struct rusage *) 0);
        if (i == pid) {
            if (WIFSTOPPED(w)) {
            /* Stop outselves, if child stops. */
                (void)kill(getpid(), SIGSTOP);
            /* Restore the control terminal, and restart subprocess. */
                if (pgrp >= 0)
                    (void)ioctl(0, TIOCSPGRP, (char *)&pid);
                (void)killpg(pid, SIGCONT);
            } else
                break;
        }
    }

    /* Get the control terminal back. */
    if (pgrp >= 0) {
        (void)ioctl(0, TIOCSPGRP, (char *)&pgrp);
    }

    (void)sigsetmask(mask);	   /* Interrupts on. */

    /* XXX This is no longer defined -- use "term.c" */
    xxx xxx xxx
    moriaterm();		   /* Terminal in moria mode. */
    xxx xxx xxx

    return 0;
}

#endif

#endif


#endif /* USE_CUR */


