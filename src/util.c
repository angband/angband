/*
 * util.c: miscellanous utilities 
 *
 * Copyright (c) 1989 James E. Wilson 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "constant.h"
#include "config.h"
#include "types.h"
#include "externs.h"

#if defined(NEEDS_USLEEP)
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

/* for those systems that don't have usleep */
/* grabbed from the inl netrek server -cba */

int
microsleep(microSeconds)
    unsigned long       microSeconds;
{
    unsigned int        Seconds, uSec;
    int                 nfds, readfds, writefds, exceptfds;
    struct timeval      Timer;

    nfds = readfds = writefds = exceptfds = 0;

    if (microSeconds > (unsigned long)4000000) {
	errno = ERANGE;		   /* value out of range */
	perror("usleep time out of range ( 0 -> 4000000 ) ");
	return -1;
    }
    Seconds = microSeconds / (unsigned long)1000000;
    uSec = microSeconds % (unsigned long)1000000;

    Timer.tv_sec = Seconds;
    Timer.tv_usec = uSec;
    if (select(nfds, &readfds, &writefds, &exceptfds, &Timer) < 0) {
	if (errno != EINTR) {
	    perror("usleep (select) failed");
	    return -1;
	}
    }
    return 0;
}

#endif
