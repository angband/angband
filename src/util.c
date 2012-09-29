/* File: util.c */

/* Purpose: miscellanous utilities */

/* Done: Ansi */
/* Todo: verify "tilde()" and other functions */

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
    unsigned int        Seconds, uSec;
    int                 nfds, readfds, writefds, exceptfds;
    struct timeval      Timer;

    nfds = readfds = writefds = exceptfds = 0;

    if (microSeconds > (unsigned long)4000000) {
	errno = ERANGE;		   /* value out of range */
	plog("usleep time out of range (0 to 4000000)");
	return -1;
    }

    Seconds = microSeconds / (unsigned long)1000000;
    uSec = microSeconds % (unsigned long)1000000;

    Timer.tv_sec = Seconds;
    Timer.tv_usec = uSec;
    if (select(nfds, &readfds, &writefds, &exceptfds, &Timer) < 0) {
	if (errno != EINTR) {
	    plog("usleep (select) failed");
	    return -1;
	}
    }
    return 0;
}

#endif




#if defined(SET_UID)

#include <pwd.h>

struct passwd      *getpwuid();
struct passwd      *getpwnam();


/*
 * Find a default user name from the system.
 * We do NOT capitalize this name.  It is probably
 * NOT a "human" name, and if it is, the player can
 * "choose" the proper name with "-uPlayername"
 */
void user_name(char *buf, int id)
{
    struct passwd *pw;

    /* Look up the user name */
    pw = getpwuid(id);
    (void)strcpy(buf, pw->pw_name);

    /* Hack -- only allow 15 letters (just in case) */
    buf[16] = '\0';
}


/*
 * Attempt to expand leading tildes at the beginning of file names
 * Replace "~user" by the home directory of the user named "user"
 * Thus "~user" refers to the home directory of "user",
 * and "~" refers to the home directory of the current user
 * Note that the empty username is considered to be the current user
 * If successful, load the result into "exp" and return "TRUE"
 * When FALSE is returned, the original file may be fine by itself.
 */
int tilde(cptr file, char *exp)
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
 * There is no "tilde" expansion on single-user machines
 */
int tilde(cptr file, char *exp)
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

    /* Try to parse the tilde */
    if (tilde(file, buf)) file = buf;

    /* Attempt to fopen the file anyway */
    return (fopen(file, mode));
}


/*
 * Replacement for "open" which parses leading tilde's
 */
int my_topen(cptr file, int flags, int mode)
{
    char                buf[1024];

    /* Try to parse the tilde */
    if (tilde(file, buf)) file = buf;

#ifdef MACINTOSH    
    /* Attempt to open the file anyway */
    /* Macintosh "open()" is brain-dead */
    return (open((char*)(file), flags));
#else
    /* Attempt to open the file anyway */
    return (open(file, flags, mode));
#endif

}


