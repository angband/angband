/* File: z-virt.c */

#include "z-virt.h"

#include "z-util.h"


/*
 * Allow debugging messages to track memory usage.
 */
#ifdef VERBOSE_RALLOC
static long ralloc_total = 0;
static long ralloc_block = 0;
#endif


/*
 * The "lifeboat" in "rpanic()" can sometimes prevent crashes
 */
#ifndef RPANIC_LIFEBOAT
# define RPANIC_LIFEBOAT 1024
#endif



#ifndef HAS_MEMSET

/*
 * Set the value of each of 'n' bytes starting at 's' to 'c', return 's'
 * If 'n' is negative, you will erase a whole lot of memory.
 */
char *memset(char *s, int c, huge n)
{
  register char *t;
  for (t = s; len--; ) *t++ = c;
  return (s);
}

#endif




/*
 * Optional auxiliary "rnfree" function
 */
errr (*rnfree_aux)(vptr, huge) = NULL;

/*
 * Free some memory (that was allocated by ralloc).
 */
errr rnfree (vptr p, huge len)
{
  /* Easy to free zero bytes */
  if (len == 0) return (0);

#ifdef VERBOSE_RALLOC

  /* Decrease memory count */
  ralloc_total -= len;

  /* Message */
  if (len > ralloc_block)
  {
    char buf[128];

    sprintf(buf, "rnfree(%ld bytes).  Allocated: %ld bytes",
	    len, ralloc_total);

    plog(buf);
  }

#endif

  /* Use the "aux" function */
  if (rnfree_aux) return ((*rnfree_aux)(p, len));

  /* Or just use "free" */
  else free ((char*)(p));

  /* Success */
  return (0);
}


/*
 * Optional auxiliary "rpanic" function
 */
vptr (*rpanic_aux)(huge) = NULL;

/*
 * The system is out of memory, so panic.  If "rpanic_aux" is set,
 * it can be used to free up some memory and do a new "ralloc()",
 * or if not, it can be used to save the state before crashing.
 * If it first unsets "rpanic_aux", then it can revert to the
 * built in rpanic.
 */
vptr rpanic(huge len)
{
  static byte lifeboat[RPANIC_LIFEBOAT];
  static huge lifesize = RPANIC_LIFEBOAT;

  /* Hopefully, we have a real "panic" function */  
  if (rpanic_aux) return ((*rpanic_aux)(len));

  /* Lifeboat is too small! */
  if (lifesize < len) return (V_NULL);

  /* We are probably going to crash anyway */
  plog("Running out of memory!!!");

  /* Hack -- decrease the lifeboat */
  lifesize -= len;

  /* Hack -- use part of the lifeboat */
  return (lifeboat + lifesize);
}


/*
 * Optional auxiliary "ralloc" function
 */
vptr (*ralloc_aux)(huge) = NULL;

/*
 * Allocate some memory
 * XXX Consider using "NewPtr()" on Macintosh.
 * XXX Then you will need to use FreePtr() below.
 */
vptr ralloc(huge len)
{
  vptr mem;

  /* Allow allocation of zero bytes */
  if (len == 0) return (V_NULL);

#ifdef VERBOSE_RALLOC

  /* Remember how much memory is allocated */
  ralloc_total += len;

  /* Log if the change was meaningful */
  if (len > ralloc_block)
  {
    /* Log a message */
    char buf[128];

    sprintf(buf, "ralloc(%ld bytes).  Allocated: %ld bytes",
	    len, ralloc_total);

    plog(buf);
  }

#endif

  /* Use the aux function if set */
  if (ralloc_aux) mem = (*ralloc_aux)(len);

  /* Use malloc() to allocate some memory */
  else mem = ((vptr)(malloc((size_t)(len))));

  /* We were able to acquire memory */
  if (mem) return (mem);

  /* If necessary, panic */
  mem = rpanic(len);

  /* We were able to acquire memory */
  if (mem) return (mem);

  /* Abort the system */
  core("OUT OF MEMORY!!!");

  /* Make the compiler happy */
  return (V_NULL);
}




/*
 * Allocate a constant string, containing the same thing as 'str'
 */
cptr string_make(cptr str)
{
  register huge len = 0;
  register cptr t = str;
  register char *s, *res;

  /* Simple sillyness */
  if (!str) return (str);

  /* Get the number of chars in the string, including terminator */
  while (str[len++]);

  /* Allocate space for the string */
  s = res = (char*)(ralloc(len));

  /* Copy the string (with terminator) */
  while ((*s++ = *t++));

  /* Return the allocated, initialized, string */
  return (res);
}


/*
 * Un-allocate a string allocated above.
 * Depends on no changes being made to the string.
 */
errr string_free(cptr str)
{
  register huge len = 0;

  /* Succeed on non-strings */
  if (!str) return (0);

  /* Count the number of chars in 'str' plus the terminator */
  while (str[len++]);

  /* Kill the buffer of chars we must have allocated above */
  rnfree((vptr)(str), len);

  /* Success */
  return (0);
}


