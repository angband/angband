/* File: h-system.h */

#ifndef INCLUDED_H_SYSTEM_H
#define INCLUDED_H_SYSTEM_H

/*
 * Include the basic "system" files.
 *
 * Make sure all "system" constants/macros are defined.
 * Make sure all "system" functions have "extern" declarations.
 *
 * This file is a big hack to make other files less of a hack.
 * This file has been rebuilt -- it may need a little more work.
 *
 * It is (very) unlikely that VMS will work without help.
 */


#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#if defined(NeXT)
# include <libc.h>
#else
# include <stdlib.h>
#endif


#if defined(SET_UID)

# include <sys/types.h>

# if defined(Pyramid) || defined(NeXT) || defined(sun) || \
    defined(NCR3K) || defined(linux) || defined(ibm032) || \
    defined(__osf__) || defined(ISC) || defined(SGI)
#  include <sys/time.h>
# endif

# if !defined(MACINTOSH) && !defined(AMIGA) && \
    !defined(sgi) && !defined(ultrix)
#  include <sys/timeb.h>
# endif

#endif


#include <time.h>



#ifdef MACINTOSH
# include <unix.h>
#endif

#ifdef MSDOS
# include <io.h>
#endif

#if defined(__MINT__)
# include <support.h>
#endif

#if !defined(MACINTOSH) && !defined(AMIGA)
# if defined(__TURBOC__) || defined(__WATCOMC__)
#  include <mem.h>
# else
#  include <memory.h>
# endif
#endif


#if !defined(NeXT) && !defined(__MWERKS__) && !defined(ATARIST_MWC)
# include <fcntl.h>
#endif


#if defined(SET_UID)

# ifndef USG
#  include <sys/param.h>
# endif

# ifndef USG
#  include <sys/file.h>
# endif

# ifdef linux
#  include <sys/file.h>
# endif

# include <pwd.h>

# include <unistd.h>

# include <sys/stat.h>

# if defined(SOLARIS)
#  include <netdb.h>
# endif

#endif



#ifdef USG
# ifdef ATARIST_MWC
   extern char *strcat();
   extern char *strcpy();
# else
#  include <string.h>
# endif
#else
# include <strings.h>
  extern char *strchr();
  extern char *strstr();
#endif


#if !defined(linux) && !defined(__MWERKS__)
  extern long atol();
#endif



#endif

