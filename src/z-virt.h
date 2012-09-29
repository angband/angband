/* File: z-virt.h */

#ifndef INCLUDED_Z_VIRT_H
#define INCLUDED_Z_VIRT_H

#include "h-include.h"

/*
 * Memory management routines.
 *
 * Set ralloc_aux to modify the memory allocation routine.
 * Set rnfree_aux to modify the memory de-allocation routine.
 * Set rpanic_aux to let the program react to memory failures.
 *
 * These routines will not work as well if the program calls malloc/free.
 *
 * The string_make() and string_free() routines handle dynamic strings.
 * A dynamic string is a string allocated at run-time, but not really
 * intended to be modified once allocated.
 *
 * Note the macros below which simplify the details of allocation,
 * clearing, casting, and size extraction.
 *
 * The macros MAKE/C_MAKE and KILL/C_KILL have a "procedural" metaphor.
 *
 * NOTE: For some reason, many allocation macros disallow "stars"
 * in type names, but you can use typedefs to circumvent this.
 * For example, instead of "type **p; MAKE(p,type*);" you can use
 * "typedef type *type_ptr; type_ptr *p; MAKE(p,type_ptr)".
 *
 * Note that it is assumed that "memset()" will function correctly,
 * in particular, that it returns its first argument.
 *
 * The 'GROW' macro is very tentative.
 */



/**** Available variables ****/

extern func_errr rnfree_aux;
extern func_vptr rpanic_aux;
extern func_vptr ralloc_aux;


/**** Available Routines ****/

/* De-allocate a given amount of memory */
extern errr rnfree(vptr p, huge len);

/* Panic, attempt to Allocate 'len' bytes */
extern vptr rpanic(huge len);

/* Allocate (and return) 'len', or dump core */
extern vptr ralloc(huge len);

/* Create a "dynamic string" */
extern cptr string_make(cptr str);

/* Free a string allocated with "string_make()" */
extern errr string_free(cptr str);

#ifndef HAS_MEMSET
extern char *memset(char*, int, huge);
#endif




/**** Memory Macros ****/


/* Size of 'N' things of type 'T' */
#define C_SIZE(N,T) \
	((huge)((N)*(sizeof(T))))

/* Size of one thing of type 'T' */
#define SIZE(T) \
	((huge)(sizeof(T)))


/* Bool: True when P1 and P2 both point to N T's, which have same contents */
#define C_SAME(P1,P2,N,T) \
	(!memcmp((char*)(P1),(char*)(P2),C_SIZE(N,T)))

/* Bool: True when P1 and P2 both point to T's, and they have same contents */
#define SAME(P1,P2,T) \
	(!memcmp((char*)(P1),(char*)(P2),SIZE(T)))


/* Wipe an array of N things of type T at location P, return T */
#define C_WIPE(P,N,T) \
	memset((char*)(P),0,C_SIZE(N,T))

/* Wipe a single thing of type T at location P, return T */
#define WIPE(P,T) \
	memset((char*)(P),0,SIZE(T))


/* When P1 and P2 both point to N T's, Load P1 from P2 explicitly */
#define C_COPY(P1,P2,N,T) \
	memcpy((char*)(P1),(char*)(P2),C_SIZE(N,T))

/* When P1 and P2 both point to T's, Load P1 from P2 explicitly */
#define COPY(P1,P2,T) \
	memcpy((char*)(P1),(char*)(P2),SIZE(T))



/* Free an array of N things of type T at P, return ??? */
#define C_FREE(P,N,T) \
	(rnfree(P,C_SIZE(N,T)))

/* Free one thing of type T at P, return ??? */
#define FREE(P,T) \
	(rnfree(P,SIZE(T)))


/* Allocate and return an array of N things of type T */
#define C_NEW(N,T) \
	((T*)(ralloc(C_SIZE(N,T))))

/* Allocate and return one thing of type T */
#define NEW(T) \
	((T*)(ralloc(SIZE(T))))


/* Allocate, wipe, and return an array of N things of type T */
#define C_ZNEW(N,T) \
	((T*)(C_WIPE(ralloc(C_SIZE(N,T)),N,T)))

/* Allocate, wipe, and return one thing of type T */
#define ZNEW(T) \
	((T*)(WIPE(ralloc(SIZE(T)),T)))


/* Allocate a wiped array of N things of type T, let P point at them */
#define C_MAKE(P,N,T) \
	(P)=C_ZNEW(N,T)

/* Allocate a wiped thing of type T, let P point at it */
#define MAKE(P,T) \
	(P)=ZNEW(T)


/* Free an array of N things of type T at P, and reset P to NULL */
#define C_KILL(P,N,T) \
	(C_FREE(P,N,T), (P)=(T*)NULL)

/* Free a single thing of type T at P, and reset P to NULL */
#define KILL(P,T) \
	(FREE(P,T), (P)=(T*)NULL)



/* Mega-Hack -- Cleanly "grow" 'P' from N1 T's to N2 T's */
#define GROW(P,N1,N2,T) \
	(C_MAKE(vptr_tmp,N2,T), C_COPY(vptr_tmp,P,MIN(N1,N2),T), \
	 C_FREE(P,N1,T), (P)=vptr_tmp)



#endif



