/* File: signals.c */

/* Purpose: signal handlers */

/*
 * Copyright (c) 1989 James E. Wilson 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

/*
 * This signal package was brought to you by		-JEW- 
 * Completely rewritten by				-CJS-
 * Rewritten again by					-BEN-
 * One more time by Jeff Collins (Jan. 1995).
 */

#include <signal.h>

#include "angband.h"

/*
 * We need a simple method to correctly refer to the signal handler
 */
#ifdef __MINT__
# define signal_handler ((__Sigfunc)(signal_handler_proc))
# define suspend_handler ((__Sigfunc)(suspend_handler_proc))
#else
# ifdef linux
#  define signal_handler ((void (*)())(signal_handler_proc))
#  define suspend_handler ((void (*)())(suspend_handler_proc))
# else
#  define signal_handler (signal_handler_proc)
#  define suspend_handler (suspend_handler_proc)
# endif
#endif


/*
 * suspend_handler_proc
 */
static void suspend_handler_proc(int sig)
{

#ifdef SIGSTOP

    /* Hack -- note suspend status */
    p_ptr->male |= 2;

    /* Suspend the "Term" */
    Term_xtra(TERM_XTRA_LEAVE);

    /* Suspend the process */
    (void)kill(0, SIGSTOP);

    /* Resume the "Term" */
    Term_xtra(TERM_XTRA_ENTER);

    /* Hack -- forget suspend status */
    p_ptr->male &= ~2;

#endif

}



/*
 * signal_handler_proc - Handle signals
 *
 * The most important aspect of this is to catch SIGINT and SIGQUIT and
 * allow the user to rethink.  We don't want to kill a character because
 * of a stupid typo.
 */

static void signal_handler_proc(int sig)
{
    int simple;

#if !defined(MACINTOSH)

    /* Ignore all second signals */
    (void)signal(sig, SIG_IGN);

    /* Simple "Ctrl-C" interupt, or "Quit" key */
    simple = (sig == SIGINT || sig == SIGQUIT);
    
    /* Handle simple interrupt */
    if (simple) {

	if (!death && !character_saved && character_generated) {

	    /* Allow player to think twice. */
	    if (!get_check(total_winner ?
			   "Do you want to retire?" :
			   "Really commit *Suicide*?")) {

		/* Disturb and clear */
		if (turn > 0) disturb(1, 0);
		erase_line(0, 0);
		Term_fresh();

		/* Have to restore handler. */
		(void)signal(sig, signal_handler);

		/* OK. We don't quit. */
		return;
	    }

	    (void)strcpy(died_from, "Interrupting");
	}
	else {
	    (void)strcpy(died_from, "Abortion");
	}

        /* Interrupted */
	prt("Interrupt!", 0, 0);
	death = TRUE;

        /* Save and exit */
	exit_game();

	/* Just in case */
	quit("interrupted");

	/* Paranoia */
	return;
    }

#endif /* !MACINTOSH */

    /* Die. */
    prt("OH NO!!!!!!  A gruesome software bug LEAPS out at you!", 22, 0);

    /* Try to save anyway */
    if (!death && !character_saved && character_generated) {

	/* Try a panic save */
	panic_save = 1;
	prt("Your guardian angel is trying to save you.", 23, 0);
	(void)sprintf(died_from, "(panic save %d)", sig);
	if (save_player()) quit("panic save succeeded");
	
	/* Oops */
	(void)strcpy(died_from, "software bug");
	death = TRUE;
	turn = 0;
    }
    else {
	death = TRUE;
	prt("There is NO defense!", 23, 60);

	/* Low level access -- Quietly save the memory anyway. */
	(void)_save_player(savefile);
    }

    /* Shut down the terminal */
    Term_nuke();

#ifdef SET_UID
    /* generate a core dump if necessary */
    (void)signal(sig, SIG_DFL);
    (void)kill(getpid(), sig);
    (void)sleep(5);
#endif

    /* Paranoia -- quit anyway */
    quit(NULL);
}

/*
 * signals_ignore_tstp - Ignore SIGTSTP signals.
 */
void signals_ignore_tstp(void)
{
#ifdef SIGTSTP
    (void)signal(SIGTSTP, SIG_IGN);
#endif
}

/*
 * signals_handle_tstp - Handle SIGTSTP (keyboard suspend)
 */
void signals_handle_tstp(void)
{
#ifdef SIGTSTP
    /* Tell "suspend" to suspend */
    (void)signal(SIGTSTP, suspend_handler);
#endif
}

/*
 * Prepare to handle the relevant signals
 */
void signals_init()
{

#ifdef SIGHUP
    /* Ignore HANGUP */
    (void)signal(SIGHUP, SIG_IGN);
#endif

#ifdef SIGTSTP
    /* Hack -- suspend gracefully */
    (void)signal(SIGTSTP, suspend_handler);
#endif

#ifdef SIGINT
    /* Catch the basic "Ctrl-C" interupt */
    (void)signal(SIGINT, signal_handler);
#endif

#ifdef SIGQUIT
    (void)signal(SIGQUIT, signal_handler);
#endif

#ifdef SIGFPE
    (void)signal(SIGFPE, signal_handler);
#endif

#ifdef SIGILL
    (void)signal(SIGILL, signal_handler);
#endif

#ifdef SIGTRAP
    (void)signal(SIGTRAP, signal_handler);
#endif

#ifdef SIGIOT
    (void)signal(SIGIOT, signal_handler);
#endif

#ifdef SIGKILL
    (void)signal(SIGKILL, signal_handler);
#endif

#ifdef SIGBUS
    (void)signal(SIGBUS, signal_handler);
#endif

#ifdef SIGSEGV
    (void)signal(SIGSEGV, signal_handler);
#endif

#ifdef SIGTERM
    (void)signal(SIGTERM, signal_handler);
#endif

#ifdef SIGPIPE
    (void)signal(SIGPIPE, signal_handler);
#endif

#ifdef SIGEMT			   /* in BSD systems */
    (void)signal(SIGEMT, signal_handler);
#endif

#ifdef SIGDANGER		   /* in SYSV systems */
    (void)signal(SIGDANGER, signal_handler);
#endif

#ifdef SIGSYS
    (void)signal(SIGSYS, signal_handler);
#endif

#ifdef SIGXCPU   /* BSD */
    (void)signal(SIGXCPU, signal_handler);
#endif

#ifdef SIGPWR   /* SYSV */
    (void)signal(SIGPWR, signal_handler);
#endif
}

