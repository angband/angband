/* File: signals.c */

/* Purpose: signal handlers (multiple revisions) */

#include "angband.h"



#ifdef HANDLE_SIGNALS


#include <signal.h>


/*
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 */
static void handle_signal_suspend(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);

#ifdef SIGSTOP

    /* Flush output */
    Term_fresh();

    /* Suspend the "Term" */
    Term_xtra(TERM_XTRA_ALIVE, 0);

    /* Suspend ourself */
    (void)kill(0, SIGSTOP);

    /* Resume the "Term" */
    Term_xtra(TERM_XTRA_ALIVE, 1);

    /* Redraw the term */
    Term_redraw();

    /* Flush the term */
    Term_fresh();

#endif

    /* Restore handler */
    (void)signal(sig, handle_signal_suspend);
}


/*
 * Handle signals -- simple (interrupt and quit)
 *
 * This function was causing a *huge* number of problems, so it has
 * been simplified greatly.  We keep a global variable which counts
 * the number of times the user attempts to kill the process, and
 * we commit suicide if the user does this a certain number of times.
 *
 * We attempt to give "feedback" to the user as he approaches the
 * suicide thresh-hold, but without penalizing accidental keypresses.
 *
 * To prevent messy accidents, we should reset this global variable
 * whenever the user enters a keypress, or something like that.
 */
static void handle_signal_simple(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);


    /* Nothing to save, just quit */
    if (!character_generated || character_saved) quit(NULL);


    /* Count the signals */
    signal_count++;

    
    /* Terminate dead characters */
    if (death) {

        /* Mark the savefile */
        (void)strcpy(died_from, "Abortion");

        /* Close stuff */
        close_game();

        /* Quit */
        quit("interrupt");
    }

    /* Allow suicide (after 5) */
    else if (signal_count >= 5) {
    
        /* Cause of "death" */
        (void)strcpy(died_from, "Interrupting");

        /* Suicide */
        death = TRUE;

        /* Close stuff */
        close_game();

        /* Quit */
        quit("interrupt");
    }

    /* Give warning (after 4) */
    else if (signal_count >= 4) {
    
        /* Make a noise */
        Term_xtra(TERM_XTRA_NOISE, 0);
    
        /* Clear the top line */
        Term_erase(0, 0, 80, 1);
        
        /* Display the cause */
        Term_putstr(0, 0, -1, TERM_WHITE, "Contemplating suicide!");

        /* Flush */
        Term_fresh();
    }

    /* Give warning (after 2) */
    else if (signal_count >= 2) {
    
        /* Make a noise */
        Term_xtra(TERM_XTRA_NOISE, 0);
    }

    /* Restore handler */
    (void)signal(sig, handle_signal_simple);
}


/*
 * Handle signal -- abort, kill, etc
 */
static void handle_signal_abort(int sig)
{
    /* Disable handler */
    (void)signal(sig, SIG_IGN);


    /* Nothing to save, just quit */
    if (!character_generated || character_saved) quit(NULL);

    
    /* Clear the bottom line */
    Term_erase(0, 23, 80, 1);
    
    /* Give a warning */
    Term_putstr(0, 23, -1, TERM_RED,
                "A gruesome software bug LEAPS out at you!");

    /* Message */
    Term_putstr(45, 23, -1, TERM_RED, "Panic save...");

    /* Flush output */
    Term_fresh();

    /* Panic Save */
    panic_save = 1;

    /* Panic save */
    (void)strcpy(died_from, "(panic save)");

    /* Attempt to save */
    if (save_player()) {
        Term_putstr(45, 23, -1, TERM_RED, "Panic save succeeded!");
    }
    
    /* Save failed */
    else {
        Term_putstr(45, 23, -1, TERM_RED, "Panic save failed!");
    }
    
    /* Flush output */
    Term_fresh();
    
    /* Quit */
    quit("software bug");
}




/*
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
    (void)signal(SIGTSTP, SIG_IGN);
#endif

}

/*
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
    (void)signal(SIGTSTP, handle_signal_suspend);
#endif

}


/*
 * Prepare to handle the relevant signals
 */
void signals_init()
{

#ifdef SIGHUP
    (void)signal(SIGHUP, SIG_IGN);
#endif


#ifdef SIGTSTP
    (void)signal(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
    (void)signal(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
    (void)signal(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
    (void)signal(SIGFPE, handle_signal_abort);
#endif

#ifdef SIGILL
    (void)signal(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
    (void)signal(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
    (void)signal(SIGIOT, handle_signal_abort);
#endif

#ifdef SIGKILL
    (void)signal(SIGKILL, handle_signal_abort);
#endif

#ifdef SIGBUS
    (void)signal(SIGBUS, handle_signal_abort);
#endif

#ifdef SIGSEGV
    (void)signal(SIGSEGV, handle_signal_abort);
#endif

#ifdef SIGTERM
    (void)signal(SIGTERM, handle_signal_abort);
#endif

#ifdef SIGPIPE
    (void)signal(SIGPIPE, handle_signal_abort);
#endif

#ifdef SIGEMT
    (void)signal(SIGEMT, handle_signal_abort);
#endif

#ifdef SIGDANGER
    (void)signal(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
    (void)signal(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
    (void)signal(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
    (void)signal(SIGPWR, handle_signal_abort);
#endif

}


#else	/* HANDLE_SIGNALS */


/*
 * Do nothing
 */
void signals_ignore_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_handle_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_init(void)
{
}


#endif	/* HANDLE_SIGNALS */

