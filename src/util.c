/* File: util.c */

/* Purpose: annoying system dependant utilities */


#include "angband.h"



#ifdef VM

/*
 * Version of "delay()" for VM
 */
void delay(int t)
{
    /* Do nothing */
}

#endif /* VM */


#ifdef AMIGA

/*
 * Version of "delay()" for AMIGA
 */
void delay(int t)
{
    if (t >= 20) Delay(t / 20);
}

#endif /* AMIGA */


#ifdef __EMX__

/*
 * Version of "delay()" for __EMX__
 */
void delay(int t)
{
    _sleep2(t);
}

#endif /* __EMX__ */





#ifndef HAS_MEMSET

/*
 * For those systems that don't have "memset()"
 *
 * Set the value of each of 'n' bytes starting at 's' to 'c', return 's'
 * If 'n' is negative, you will erase a whole lot of memory.
 */
char *memset(char *s, int c, huge n)
{
  char *t;
  for (t = s; len--; ) *t++ = c;
  return (s);
}

#endif



#ifndef HAS_STRICMP

/*
 * For those systems that don't have "stricmp()"
 *
 * Compare the two strings "a" and "b" ala "strcmp()" ignoring case.
 */
int stricmp(cptr a, cptr b)
{
  cptr s1, s2;
  char z1, z2;

  /* Scan the strings */
  for (s1 = a, s2 = b; TRUE; s1++, s2++)
  {
    z1 = FORCEUPPER(*s1);
    z2 = FORCEUPPER(*s2);
    if (z1 < z2) return (-1);
    if (z1 > z2) return (1);
    if (!z1) return (0);
  }
}

#endif


#ifdef SET_UID

# ifndef HAS_USLEEP

/*
 * For those systems that don't have "usleep()" but need it.
 *
 * Fake "usleep()" function grabbed from the inl netrek server -cba
 */
static int usleep(huge microSeconds)
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

# endif


/*
 * Version of "delay()" for Unix machines
 */
void delay(int t)
{
    /* Do it in micro-seconds */
    usleep(1000 * t);
}


/*
 * Hack -- External functions
 */
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();


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
        /* Hack -- capitalize the user name */
        if (islower(buf[0])) buf[0] = toupper(buf[0]);
#endif

        return;
    }

    /* Oops.  Hack -- default to "PLAYER" */
    strcpy(buf, "PLAYER");
}

#endif /* SET_UID */


#ifdef ACORN

/*
 * All the "file" routines for "ACORN" are in "main-acn.c"
 */

#else /* ACORN */

#ifdef SET_UID

/*
 * Attempt to expand leading tilde's at the beginning of file names
 * Replace "~user" by the home directory of the user named "user"
 * Thus "~user" refers to the home directory of "user",
 * and "~" refers to the home directory of the current user
 * Note that the empty username is considered to be the current user
 * If successful, load the result into "exp" and return "TRUE"
 * When FALSE is returned, the original file may be fine by itself.
 */
static bool parse_path(cptr file, char *exp)
{
    cptr	u, s;
    struct passwd	*pw;
    char		user[128];


    /* Assume no result */
    exp[0] = '\0';

    /* No file? */
    if (!file) return (FALSE);

    /* No tilde? */
    if (file[0] != '~') return (FALSE);

    /* Point at the user */
    u = file+1;

    /* Look for non-user portion of the file */
    s = strstr(u, PATH_SEP);

    /* Hack -- no long user names */
    if (s && (s >= u + sizeof(user))) return (FALSE);

    /* Extract a user name */
    if (s) {
        int i;
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
    if (!pw) return (FALSE);

    /* Make use of the info */
    (void)strcpy(exp, pw->pw_dir);

    /* Append the rest of the filename, if any */
    if (s) (void)strcat(exp, s);

    /* Success */
    return (TRUE);
}


#else

/*
 * There is no "expansion" on single-user machines
 */
static bool parse_path(cptr file, char *exp)
{
    /* Always fails */
    return (FALSE);
}

#endif



/*
 * The Macintosh is a little bit brain-dead sometimes
 */

#ifdef MACINTOSH

# undef open
# define open(N,F,M) open((char*)(N),F)

# undef write
# define write(F,B,S) write(F,(char*)(B),S)

#endif /* MACINTOSH */


/*
 * XXX XXX XXX Hack -- replacement for "fopen()"
 */
FILE *my_fopen(cptr file, cptr mode)
{
    char                buf[1024];

    /* Hack -- Try to parse the path */
    if (parse_path(file, buf)) file = buf;

    /* Attempt to fopen the file anyway */
    return (fopen(file, mode));
}


/*
 * XXX XXX XXX Hack -- replacement for "fclose()"
 */
errr my_fclose(FILE *fff)
{
    /* Close, check for error */
    if (fclose(fff) == EOF) return (1);
    
    /* Success */
    return (0);
}




/*
 * Hack -- attempt to open a file descriptor
 */
int fd_open(cptr file, int flags, int mode)
{
    char                buf[1024];

    /* Hack -- Try to parse the path */
    if (parse_path(file, buf)) file = buf;

    /* Attempt to open the file */
    return (open(file, flags, mode));
}


/*
 * Hack -- attempt to lock a file descriptor
 *
 * Legal lock types -- F_UNLCK, F_RDLCK, F_WRLCK
 */
errr fd_lock(int fd, int what)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

#ifdef SET_UID

# ifdef USG

    /* Un-Lock */
    if (what == F_UNLCK) {

        /* Unlock it, Ignore errors */
        lockf(fd, F_ULOCK, 0);
    }
    
    /* Lock */
    else {

        /* Lock the score file */
        if (lockf(fd, F_LOCK, 0) != 0) return (1);
    }

#else

    /* Un-Lock */
    if (what == F_UNLCK) {

        /* Unlock it, Ignore errors */
        (void)flock(fd, LOCK_UN);
    }
    
    /* Lock */
    else {

        /* Lock the score file */
        if (flock(fd, LOCK_EX) != 0) return (1);
    }

# endif

#endif

    /* Success */
    return (0);
}


/*
 * Hack -- attempt to seek on a file descriptor
 */
errr fd_seek(int fd, huge n)
{
    long p;

    /* Verify fd */
    if (fd < 0) return (-1);
        
    /* Seek to the given position */
    p = lseek(fd, n, SEEK_SET);
    
    /* Failure */
    if (p < 0) return (1);
    
    /* Failure */
    if (p != n) return (1);
    
    /* Success */
    return (0);
}


#if 0

/*
 * Hack -- attempt to truncate a file descriptor
 */
errr fd_chop(int fd, huge n)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

#if defined(sun) || defined(ultrix) || defined(NeXT)
    /* Truncate */
    ftruncate(fd, n);
#endif

    /* Success */
    return (0);
}

#endif


/*
 * Hack -- attempt to read data from a file descriptor
 */
errr fd_read(int fd, char *buf, huge n)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

#ifndef SET_UID

    /* Read pieces */
    while (n >= 16384) {

        /* Read a piece */
        if (read(fd, buf, 16384) != 16384) return (1);
        
        /* Shorten the task */
        buf += 16384;

        /* Shorten the task */
        n -= 16384;
    }

#endif

    /* Read the final piece */
    if (read(fd, buf, n) != n) return (1);

    /* Success */
    return (0);
}


/*
 * Hack -- Attempt to write data to a file descriptor
 */
errr fd_write(int fd, cptr buf, huge n)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

#ifndef SET_UID

    /* Write pieces */
    while (n >= 16384) {

        /* Write a piece */
        if (write(fd, buf, 16384) != 16384) return (1);
        
        /* Shorten the task */
        buf += 16384;

        /* Shorten the task */
        n -= 16384;
    }

#endif

    /* Write the final piece */
    if (write(fd, buf, n) != n) return (1);

    /* Success */
    return (0);
}


/*
 * Hack -- attempt to close a file descriptor
 */
errr fd_close(int fd)
{
    /* Verify the fd */
    if (fd < 0) return (-1);

    /* Close XXX XXX XXX check error */
    close(fd);
    
    /* Success */
    return (0);
}

#endif /* ACORN */
