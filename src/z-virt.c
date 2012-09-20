/* File: z-virt.c */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: Memory management routines -BEN- */

#include "z-virt.h"

#include "z-util.h"


/*
 * Allow debugging messages to track memory usage.
 */
#ifdef VERBOSE_RALLOC
static long virt_make = 0;
static long virt_kill = 0;
static long virt_size = 0;
#endif


/*
 * Optional auxiliary "rnfree" function
 */
vptr (*rnfree_aux)(vptr, huge) = NULL;

/*
 * Free some memory (allocated by ralloc), return NULL
 */
vptr rnfree(vptr p, huge len)
{
	/* Easy to free zero bytes */
	if (len == 0) return (NULL);

#ifdef VERBOSE_RALLOC

	/* Decrease memory count */
	virt_kill += len;

	/* Message */
	if (len > virt_size)
	{
		char buf[80];
		sprintf(buf, "Kill (%ld): %ld - %ld = %ld.",
		        len, virt_make, virt_kill, virt_make - virt_kill);
		plog(buf);
	}

#endif

	/* Use the "aux" function */
	if (rnfree_aux) return ((*rnfree_aux)(p, len));

	/* Use "free" */
	free ((char*)(p));

	/* Done */
	return (NULL);
}


/*
 * Optional auxiliary "rpanic" function
 */
vptr (*rpanic_aux)(huge) = NULL;

/*
 * The system is out of memory, so panic.  If "rpanic_aux" is set,
 * it can be used to free up some memory and do a new "ralloc()",
 * or if not, it can be used to save things, clean up, and exit.
 * By default, this function simply crashes the computer.
 */
vptr rpanic(huge len)
{
	/* Hopefully, we have a real "panic" function */
	if (rpanic_aux) return ((*rpanic_aux)(len));

	/* Attempt to crash before icky things happen */
	core("Out of Memory!");

	/* Paranoia */
	return ((vptr)(NULL));
}


/*
 * Optional auxiliary "ralloc" function
 */
vptr (*ralloc_aux)(huge) = NULL;


/*
 * Allocate some memory
 */
vptr ralloc(huge len)
{
	vptr mem;

	/* Allow allocation of "zero bytes" */
	if (len == 0) return ((vptr)(NULL));

#ifdef VERBOSE_RALLOC

	/* Count allocated memory */
	virt_make += len;

	/* Log important allocations */
	if (len > virt_size)
	{
		char buf[80];
		sprintf(buf, "Make (%ld): %ld - %ld = %ld.",
		        len, virt_make, virt_kill, virt_make - virt_kill);
		plog(buf);
	}

#endif

	/* Use the aux function if set */
	if (ralloc_aux) mem = (*ralloc_aux)(len);

	/* Use malloc() to allocate some memory */
	else mem = ((vptr)(malloc((size_t)(len))));

	/* We were able to acquire memory */
	if (!mem) mem = rpanic(len);

	/* Return the memory, if any */
	return (mem);
}




/*
 * Allocate a constant string, containing the same thing as 'str'
 */
cptr string_make(cptr str)
{
	huge len = 0;
	cptr t = str;
	char *s, *res;

	/* Simple sillyness */
	if (!str) return (str);

	/* Get the number of chars in the string, including terminator */
	while (str[len++]) /* loop */;

	/* Allocate space for the string */
	s = res = (char*)(ralloc(len));

	/* Copy the string (with terminator) */
	while ((*s++ = *t++) != 0) /* loop */;

	/* Return the allocated, initialized, string */
	return (res);
}


/*
 * Un-allocate a string allocated above.
 * Depends on no changes being made to the string.
 */
errr string_free(cptr str)
{
	huge len = 0;

	/* Succeed on non-strings */
	if (!str) return (0);

	/* Count the number of chars in 'str' plus the terminator */
	while (str[len++]) /* loop */;

	/* Kill the buffer of chars we must have allocated above */
	rnfree((vptr)(str), len);

	/* Success */
	return (0);
}


