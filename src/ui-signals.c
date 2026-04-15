/**
 * \file ui-signals.c
 * \brief Handle various OS signals
 *
 * Copyright (c) 1997 Ben Harrison
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "game-world.h"
#include "savefile.h"
#include "ui-game.h"
#include "ui-signals.h"
#include "ui-term.h"

int16_t signal_count;	/* Count interrupts ("I'm going to count to five") */

#ifndef WINDOWS

#include <signal.h>

#ifdef UNIX
# include <sys/types.h>
#endif


typedef void (*Signal_Handler_t)(int);

static int install_handler(int sig, Signal_Handler_t handler)
{
#ifdef HAVE_SIGACTION
	struct sigaction a;

	a.sa_handler = handler;
	(void)sigemptyset(&a.sa_mask);
#ifdef SA_RESTART
	a.sa_flags = SA_RESTART;
#else
	a.sa_flags = 0;
#endif
	return sigaction(sig, &a, NULL);
#else
	return (signal(sig, handler) == SIG_ERR) ? -1 : 0;
#endif
}


#ifdef SIGHUP
/**
 * Handle signals -- disconnect in an orderly fashion
 */
static void handle_signal_disconnect(int sig)
{
	terms_disconnecting = 1;
}
#endif


#ifdef SIGTSTP
/**
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 */
static void handle_signal_suspend(int sig)
{
	/* Protect errno from library calls in signal handler */
	int save_errno = errno;

#ifndef HAVE_SIGACTION
	/* Disable handler */
	(void)install_handler(sig, SIG_IGN);
#endif

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

#ifndef HAVE_SIGACTION
	/* Restore handler */
	(void)install_handler(sig, handle_signal_suspend);
#endif

	/* Restore errno */
	errno = save_errno;
}
#endif /* ifdef SIGTSTP */


/**
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
	/* Protect errno from library calls in signal handler */
	int save_errno = errno;
	/*
	 * Use own buffer to avoid interactions with the static variables
	 * used to implement vformat() (and thus quit_fmt() and format()).
	 */
	char msg[48];

#ifndef HAVE_SIGACTION
	/* Disable handler */
	(void)install_handler(sig, SIG_IGN);
#endif

	/* Construct the exit message in case it is needed */
	(void)strnfmt(msg, sizeof(msg), "Exiting on signal %d!", sig);

	/* Nothing to save, just quit */
	if (!character_generated || character_saved) quit(msg);

	/* Count the signals */
	signal_count++;

	/*
	 * Terminate dead characters; quit without saving (non-setgid
	 * installations) or suicide (setgid installations) from interrupts
	 * (after warnings)
	 */
	if (player->is_dead) {
		/* Mark the savefile */
		my_strcpy(player->died_from, "Abortion", sizeof(player->died_from));

		close_game(false);

		/* Quit */
		quit(msg);
	} else if (signal_count >= 5) {
#ifdef SETGID
		/* Cause of "death" */
		my_strcpy(player->died_from, "Interrupting", sizeof(player->died_from));

		/* Commit suicide */
		player->is_dead = true;

		/* Stop playing */
		player->upkeep->playing = false;

		/* Close stuff */
		close_game(false);
#endif

		/* Quit */
		quit(msg);
	} else if (signal_count >= 4) {
		/*
		 * Remember where the cursor was so it can be restored after
		 * the message is displayed.
		 */
		int cx, cy;
		errr cbad = Term_locate(&cx, &cy);

		/* Make a noise */
		Term_xtra(TERM_XTRA_NOISE, 0);

		/* Clear the top line */
		Term_erase(0, 0, 255);

		/* Display the cause */
#ifdef SETGID
		Term_putstr(0, 0, -1, COLOUR_WHITE, "Another interrupt (CTRL-c) will kill your character!");
#else
		Term_putstr(0, 0, -1, COLOUR_WHITE, "Another interrupt (CTRL-c) will quit without saving!");
#endif

		/* Restore the cursor position. */
		if (!cbad) {
			Term_gotoxy(cx, cy);
		}

		/* Flush */
		Term_fresh();
	} else if (signal_count >= 2) {
		/* Make a noise */
		Term_xtra(TERM_XTRA_NOISE, 0);
	}

#ifndef HAVE_SIGACTION
	/* Restore handler */
	(void)install_handler(sig, handle_signal_simple);
#endif

	/* Restore errno */
	errno = save_errno;
}


/**
 * Handle signal -- abort, kill, etc
 */
static void handle_signal_abort(int sig)
{
	/*
	 * Use own buffer to avoid interactions with the static variables
	 * used to implement vformat() (and thus quit_fmt() and format()).
	 */
	char msg[48];

#ifndef HAVE_SIGACTION
	/* Disable handler */
	(void)install_handler(sig, SIG_IGN);
#endif

	/* Construct the exit message */
	(void)strnfmt(msg, sizeof(msg), "Exiting on signal %d!", sig);

	/* Nothing to save, just quit */
	if (!character_generated || character_saved) quit(msg);

	/* Clear the bottom line */
	Term_erase(0, 23, 255);

	/* Give a warning */
	Term_putstr(0, 23, -1, COLOUR_RED,
	            "A gruesome software bug LEAPS out at you!");

	/* Message */
	Term_putstr(45, 23, -1, COLOUR_RED, "Panic save...");

	/* Flush output */
	Term_fresh();

	/* Panic save */
	my_strcpy(player->died_from, "(panic save)", sizeof(player->died_from));
	savefile_get_panic_name(panicfile, sizeof(panicfile), savefile);

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Attempt to save */
	if (panicfile[0] && savefile_save(panicfile))
		Term_putstr(45, 23, -1, COLOUR_RED, "Panic save succeeded!");
	else
		Term_putstr(45, 23, -1, COLOUR_RED, "Panic save failed!");

	/* Flush output */
	Term_fresh();

	/* Quit */
	quit(msg);
}




/**
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
	(void)install_handler(SIGTSTP, SIG_IGN);
#endif

}

/**
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
	(void)install_handler(SIGTSTP, handle_signal_suspend);
#endif

}


/**
 * Prepare to handle the relevant signals
 */
void signals_init(bool hup_disconnects)
{

#ifdef SIGHUP
	(void)install_handler(SIGHUP,
		(hup_disconnects) ? handle_signal_disconnect : SIG_IGN);
#else
	(void)hup_disconnects;
#endif


#ifdef SIGTSTP
	(void)install_handler(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
	(void)install_handler(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
	(void)install_handler(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
	(void)install_handler(SIGFPE, handle_signal_abort);
#endif

#ifdef SIGILL
	(void)install_handler(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
	(void)install_handler(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
	(void)install_handler(SIGIOT, handle_signal_abort);
#endif

/* Set to 0 to suppress signal handlers when debugging */
#if 1
# ifdef SIGBUS
	(void)install_handler(SIGBUS, handle_signal_abort);
# endif

# ifdef SIGSEGV
	(void)install_handler(SIGSEGV, handle_signal_abort);
# endif
#endif

#ifdef SIGTERM
	(void)install_handler(SIGTERM, handle_signal_abort);
#endif

#ifdef SIGPIPE
	(void)install_handler(SIGPIPE, handle_signal_abort);
#endif

#ifdef SIGEMT
	(void)install_handler(SIGEMT, handle_signal_abort);
#endif

/**
 * SIGDANGER:
 * This is not a common (POSIX, SYSV, BSD) signal, it is used by AIX(?) to
 * signal that the system will soon be out of memory.
 */
#ifdef SIGDANGER
	(void)install_handler(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
	(void)install_handler(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
	(void)install_handler(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
	(void)install_handler(SIGPWR, handle_signal_abort);
#endif

}


#else	/* !WINDOWS */


/**
 * Do nothing
 */
void signals_ignore_tstp(void)
{
}

/**
 * Do nothing
 */
void signals_handle_tstp(void)
{
}

/**
 * Do nothing
 */
void signals_init(bool hup_disconnects)
{
	(void)hup_disconnects;
}

#endif	/* !WINDOWS */

