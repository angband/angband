/* File: util.c */

/* Purpose: miscellanous utilities */

/*
 * Copyright (c) 1989 James E. Wilson 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"



#ifndef HAS_USLEEP

/*
 * for those systems that don't have usleep
 * grabbed from the inl netrek server -cba 
 * I think we include too many files above!
 */
int usleep(huge microSeconds)
{
    struct timeval      Timer;

    int                 nfds = 0;

#ifdef FD_SET
    fd_set		*no_fds = NULL;
#else
    int			*no_fds = NULL;
#endif


    /* Was: int readfds, writefds, exceptfds; */
    /* Was: readfds = writefds = exceptfds = 0; */


    /* Paranoia -- No excessive sleeping */
    if (microSeconds > 4000000L) core("Illegal usleep() call");
    

    /* Wait for it */
    Timer.tv_sec = (microSeconds / 1000000L);
    Timer.tv_usec = (microSeconds % 1000000L);

    /* Wait for it */
    if (select(nfds, no_fds, no_fds, no_fds, &Timer) < 0) {

	/* Hack -- ignore interrupts */
	if (errno != EINTR) return -1;
    }
    
    /* Success */
    return 0;
}

#endif


#ifdef MACINTOSH

/* See "main-mac.c" */

#else

#ifdef AMIGA

void delay(int t)
{
    if (t >= 20) Delay(t / 20);
}

#else

#ifdef __EMX__

void delay(int x)
{
    _sleep2(x);
}

#else

#ifndef MSDOS

/*
 * Unix port for "delay"
 */
void delay(int x)
{
    /* Do it in micro-seconds */
    usleep(1000 * x);
}

#endif	/* MSDOS */

#endif	/* __EMX__ */

#endif	/* AMIGA */

#endif	/* MACINTOSH */


#ifdef AMIGA

/*
 * Is this actually used?
 */
int getuid()
{
  return 0;
}

/*
 * Is this actually used?
 */
void umask(int x)
{
}

#endif /* AMIGA */




#if defined(SET_UID)

#include <pwd.h>

struct passwd      *getpwuid();
struct passwd      *getpwnam();


/*
 * Find a default user name from the system.
 */
void user_name(char *buf, int id)
{
    struct passwd *pw;

    /* Look up the user name */
    if ((pw = getpwuid(id))) {
	(void)strcpy(buf, pw->pw_name);
	buf[16] = '\0';

#ifdef CAPITALIZE_USER_NAME
	if (islower(buf[0])) buf[0] = toupper(buf[0]);
#endif

	return;
    }

    /* Oops.  Hack -- default to "PLAYER" */
    strcpy(buf, "PLAYER");
}


/*
 * Attempt to expand leading tilde's at the beginning of file names
 * Replace "~user" by the home directory of the user named "user"
 * Thus "~user" refers to the home directory of "user",
 * and "~" refers to the home directory of the current user
 * Note that the empty username is considered to be the current user
 * If successful, load the result into "exp" and return "TRUE"
 * When FALSE is returned, the original file may be fine by itself.
 */
static int parse_path(cptr file, char *exp)
{
    register cptr	u, s;
    struct passwd	*pw;
    char		user[128];


    /* Assume no result */
    exp[0] = '\0';

    /* No file? */    
    if (!file) return (0);

    /* No tilde? */
    if (file[0] != '~') return (0);

    /* Point at the user */
    u = file+1;

    /* Look for non-user portion of the file */
    s = strstr(u, PATH_SEP);

    /* Hack -- no long user names */
    if (s && (s >= u + sizeof(user))) return (0);

    /* Extract a user name */
    if (s) {
	register int i;
	for (i = 0; u < s; ++i) user[i] = *u++;
	user[i] = '\0';
	u = user;
    }

    /* Look up the "current" user */
    if (u[0] == '\0') u = getlogin();

    /* Look up a user (or "current" user) */
    if (u) pw = getpwnam(u);
    else pw = getpwuid(getuid());

    /* Nothing found? */
    if (!pw) return (0);

    /* Make use of the info */
    (void)strcpy(exp, pw->pw_dir);

    /* Append the rest of the filename, if any */
    if (s) (void)strcat(exp, s);

    /* Success */
    return 1;
}


#else

/*
 * No default user name
 */
void user_name(char *buf, int id)
{
    /* No name */
    buf[0] = '\0';
}

/*
 * There is no expansion on single-user machines
 */
static int parse_path(cptr file, char *exp)
{
    /* Always fails */
    return (0);
}

#endif



/*
 * Replacement for "fopen" which parses leading tilde's
 */
FILE *my_tfopen(cptr file, cptr mode)
{
    char                buf[1024];

    /* Try to parse the path */
    if (parse_path(file, buf)) file = buf;

    /* Attempt to fopen the file anyway */
    return (fopen(file, mode));
}


/*
 * Replacement for "open" which parses leading tilde's
 */
int my_topen(cptr file, int flags, int mode)
{
    char                buf[1024];

    /* Try to parse the path */
    if (parse_path(file, buf)) file = buf;

#ifdef MACINTOSH    
    /* Attempt to open the file anyway */
    /* Macintosh "open()" is brain-dead */
    return (open((char*)(file), flags));
#else
    /* Attempt to open the file anyway */
    return (open(file, flags, mode));
#endif

}


