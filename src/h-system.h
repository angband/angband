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


#ifdef SET_UID

# include <sys/types.h>

# if defined(Pyramid) || defined(NeXT) || defined(sun) || \
     defined(NCR3K) || defined(linux) || defined(ibm032) || \
     defined(__osf__) || defined(ISC) || defined(SGI)
#  include <sys/time.h>
# endif

# if !defined(sgi) && !defined(ultrix)
#  include <sys/timeb.h>
# endif

#endif


#include <time.h>



#ifdef MACINTOSH
# include <unix.h>
#endif

#if defined(WINDOWS) || defined(MSDOS)
# include <io.h>
#endif

#if !defined(MACINTOSH) && !defined(AMIGA) && \
    !defined(ACORN) && !defined(VM)
# if defined(__TURBOC__) || defined(__WATCOMC__)
#  include <mem.h>
# else
#  include <memory.h>
# endif
#endif


#if !defined(NeXT) && !defined(__MWERKS__) && !defined(ACORN)
# include <fcntl.h>
#endif


#ifdef SET_UID

# ifndef USG
#  include <sys/param.h>
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


#ifdef SET_UID

# ifdef USG
#  include <string.h>
# else
#  include <strings.h>
   extern char *strchr();
   extern char *strstr();
# endif

#else

# include <string.h>

#endif



#if !defined(linux) && !defined(__MWERKS__) && !defined(ACORN)
  extern long atol();
#endif


#include <stdarg.h>


#ifdef ACORN

/*
 * Hack -- ACORN replacement for the "rename" function
 */

#define rename(a,b) rename_acn(a,b)

extern int rename_acn(const char *old, const char *new);

/*
 * Hack -- ACORN replacement for the "remove" function
 */

#define remove(a)   remove_acn(a)

extern int remove_acn(const char *filename);

/*
 * XXX XXX XXX What about the "unlink" function?
 */

#endif /* ACORN */


#endif

