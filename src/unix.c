/*
 * unix.c: UNIX dependent code.					-CJS- 
 *
 * Copyright (c) 1989 James E. Wilson, Christopher J. Stuart 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#if defined(unix) || defined(__MINT__)

/* defines NULL */
#include <stdio.h>
/* defines CTRL */
#include <sys/ioctl.h>

/* defines TRUE and FALSE */
#ifdef linux
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <pwd.h>
#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#if defined(SYS_V) && defined(lint)
/* for AIX, prevent hundreds of unnecessary lint errors, must define before
 * signal.h is included 
 */
#define _h_IEEETRAP
typedef struct {
    int                 stuff;
}                   fpvmach;

#endif

#include <signal.h>

#ifdef M_XENIX
#include <sys/select.h>
#endif

#ifndef USG
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/param.h>
#endif

#ifdef USG
#include <string.h>
#ifndef __MINT__
#include <termio.h>
#endif
#include <fcntl.h>
#else
#include <strings.h>
#if defined(atarist) && defined(__GNUC__) && !defined(__MINT__)
/* doesn't have <sys/wait.h> */
#else
#include <sys/wait.h>
#endif
#endif

/* #include <pwd.h> */
#include <sys/errno.h>

struct passwd      *getpwuid();
struct passwd      *getpwnam();

#if defined(SYS_V) && defined(lint)
struct screen {
    int                 dumb;
};

#endif

/* Fooling lint. Unfortunately, c defines all the TIO constants to be long,
 * and lint expects them to be int. Also, ioctl is sometimes called with just
 * two arguments. The following definition keeps lint happy. It may need to
 * be reset for different systems. 
 */
#ifdef lint
#ifdef Pyramid
/* Pyramid makes constants greater than 65535 into long! Gakk! -CJS- */
/* ARGSUSED */
/* VARARGS2 */
static 
    Ioctl(i, l, p) long l;
    char               *p;
{
    return 0;
}

#else
/* ARGSUSED */
/* VARARGS2 */
static 
    Ioctl(i, l, p) char *p;
{
    return 0;
}

#endif
#define ioctl	    Ioctl
#endif

/* Provides for a timeout on input. Does a non-blocking read, consuming the
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
 */
int 
check_input(microsec)
    int                 microsec;
{
#if defined(USG) && !defined(M_XENIX)
    int                 arg, result;

#else
    struct timeval      tbuf;
    int                 ch;

#if defined(BSD4_3) || defined(M_XENIX) || defined(linux)
    fd_set              smask;

#else
    int                 smask;

#endif
#endif

/* Return true if a read on descriptor 1 will not block. */
#if !defined(USG) || defined(M_XENIX)
    tbuf.tv_sec = 0;
    tbuf.tv_usec = microsec;
#if defined(BSD4_3) || defined(M_XENIX) || defined(linux)
    FD_ZERO(&smask);
    FD_SET(fileno(stdin), &smask);
    if (select(1, &smask, (fd_set *) 0, (fd_set *) 0, &tbuf) == 1)
#else
    smask = 1;			   /* i.e. (1 << 0) */
    if (select(1, &smask, (int *)0, (int *)0, &tbuf) == 1)
#endif
    {
	ch = getch();
    /* check for EOF errors here, select sometimes works even when EOF */
	if (ch == -1) {
	    eof_flag++;
	    return 0;
	}
	return 1;
    } else
	return 0;
#else				   /* SYS V code follows */
    if (microsec != 0 && (turn & 0x7F) == 0)
	(void)sleep(1);		   /* mod 128, sleep one sec every 128 turns */
/* Can't check for input, but can do non-blocking read, so... */
/* Ugh! */
    arg = 0;
    arg = fcntl(0, F_GETFL, arg);
    arg |= O_NDELAY;
    (void)fcntl(0, F_SETFL, arg);

    result = getch();

    arg = 0;
    arg = fcntl(0, F_GETFL, arg);
    arg &= ~O_NDELAY;
    (void)fcntl(0, F_SETFL, arg);
    if (result == -1)
	return 0;
    else
	return 1;
#endif
}

#if 0
/*
 * This is not used, however, this should be compared against shell_out in
 * io.c 
 */

/*
 * A command for the operating system. Standard library function 'system' is
 * unsafe, as it leaves various file descriptors open. This also is very
 * careful with signals and interrupts, and does rudimentary job control, and
 * puts the terminal back in a standard mode. 
 */
int 
system_cmd(p)
    char               *p;
{
    int                 pgrp, pid, i, mask;
    union wait          w;
    extern char        *getenv();

    mask = sigsetmask(~0);	   /* No interrupts. */
    restore_term();		   /* Terminal in original state. */
/* Are we in the control terminal group? */
    if (ioctl(0, TIOCGPGRP, (char *)&pgrp) < 0 || pgrp != getpgrp(0))
	pgrp = (-1);
    pid = fork();
    if (pid < 0) {
	(void)sigsetmask(mask);
	moriaterm();
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
	} else
	    execl("/bin/sh", "sh", "-c", p, 0);
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
    if (pgrp >= 0)
	(void)ioctl(0, TIOCSPGRP, (char *)&pgrp);
    (void)sigsetmask(mask);	   /* Interrupts on. */
    moriaterm();		   /* Terminal in moria mode. */
    return 0;
}

#endif


/* Find a default user name from the system. */
void 
user_name(buf, id)
    char               *buf;
    int id;
{
    struct passwd      *pwd;

    pwd = getpwuid(id);
    (void)strcpy(buf, pwd->pw_name);
    if (*buf >= 'a' && *buf <= 'z')
	*buf = (*buf - 'a') + 'A';
}

/* expands a tilde at the beginning of a file name to a users home directory */
int 
tilde(file, exp)
    const char   *file;
    char         *exp;
{
    *exp = '\0';
    if (file) {
	if (*file == '~') {
	    char                user[128];
	    struct passwd      *pw = NULL;
	    int                 i = 0;

	    user[0] = '\0';
	    file++;
	    while (*file != '/' && i < sizeof(user))
		user[i++] = *file++;
	    user[i] = '\0';
	    if (i == 0) {
		char               *login = (char *)getlogin();

		if (login != NULL)
		    (void)strcpy(user, login);
		else if ((pw = getpwuid(getuid())) == NULL)
		    return 0;
	    }
	    if (pw == NULL && (pw = getpwnam(user)) == NULL)
		return 0;
	    (void)strcpy(exp, pw->pw_dir);
	}
	(void)strcat(exp, file);
	return 1;
    }
    return 0;
}

/*
 * open a file just as does fopen, but allow a leading ~ to specify a home
 * directory 
 */
FILE               *
my_tfopen(file, mode)
    const char               *file;
    const char               *mode;
{
    char                buf[1024];
    extern int          errno;

    if (tilde(file, buf))
	return (fopen(buf, mode));
    errno = ENOENT;
    return NULL;
}

/*
 * open a file just as does open, but expand a leading ~ into a home
 * directory name 
 */
int 
my_topen(file, flags, mode)
    const char               *file;
    int                 flags, mode;
{
    char                buf[1024];
    extern int          errno;

    if (tilde(file, buf))
	return (open(buf, flags, mode));
    errno = ENOENT;
    return -1;
}

#endif
