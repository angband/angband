/* File: z-virt.h */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_VIRT_H
#define INCLUDED_Z_VIRT_H

#include "h-basic.h"

/*
 * Memory management routines.
 *
 * Set ralloc_aux to modify the memory allocation routine.
 * Set rnfree_aux to modify the memory de-allocation routine.
 * Set rpanic_aux to let the program react to memory failures.
 *
 * These routines work best as a *replacement* for malloc/free.
 *
 * The string_make() and string_free() routines handle dynamic strings.
 * A dynamic string is a string allocated at run-time, which should not
 * be modified once it has been created.
 *
 * Note the macros below which simplify the details of allocation,
 * deallocation, setting, clearing, casting, size extraction, etc.
 *
 * The macros MAKE/C_MAKE and KILL have a "procedural" metaphor,
 * and they actually modify their arguments.
 *
 * Note that, for some reason, some allocation macros may disallow
 * "stars" in type names, but you can use typedefs to circumvent
 * this.  For example, instead of "type **p; MAKE(p,type*);" you
 * can use "typedef type *type_ptr; type_ptr *p; MAKE(p,type_ptr)".
 *
 * Note that it is assumed that "memset()" will function correctly,
 * in particular, that it returns its first argument.
 */



/**** Available macros ****/



/* Wipe an array of type T[N], at location P, and return P */
#define C_WIPE(P, N, T) \
	(memset((P), 0, (N) * sizeof(T)))

/* Wipe a thing of type T, at location P, and return P */
#define WIPE(P, T) \
	(memset((P), 0, sizeof(T)))


/* Load an array of type T[N], at location P1, from another, at location P2 */
#define C_COPY(P1, P2, N, T) \
	(memcpy((P1), (P2), (N) * sizeof(T)))

/* Load a thing of type T, at location P1, from another, at location P2 */
#define COPY(P1, P2, T) \
	(memcpy((P1), (P2), sizeof(T)))


/* Allocate, and return, an array of type T[N] */
#define C_RNEW(N, T) \
	(mem_alloc((N) * sizeof(T)))

/* Allocate, and return, a thing of type T */
#define RNEW(T) \
	(mem_alloc(sizeof(T)))


/* Allocate, wipe, and return an array of type T[N] */
#define C_ZNEW(N, T) \
	(C_WIPE(C_RNEW(N, T), N, T))

/* Allocate, wipe, and return a thing of type T */
#define ZNEW(T) \
	(WIPE(RNEW(T), T))


/* Free one thing at P, return NULL */
#define FREE(P) (mem_free(P), P = NULL)



/*** Initialisation bits ***/

/* Hook types for memory allocation */
typedef void *(*mem_alloc_hook)(size_t);
typedef void *(*mem_free_hook)(void *);
typedef void *(*mem_realloc_hook)(void *, size_t);

/* Set up memory allocation hooks */
bool mem_set_hooks(mem_alloc_hook alloc, mem_free_hook free, mem_realloc_hook realloc);


/**** Normal bits ***/

/* Allocate (and return) 'len', or quit */
void *mem_alloc(size_t len);

/* De-allocate memory */
void *mem_free(void *p);

/* Reallocate memory */
void *mem_realloc(void *p, size_t len);


/* Create a "dynamic string" */
char *string_make(const char *str);

/* Free a string allocated with "string_make()" */
char *string_free(char *str);
#define string_free mem_free

#endif /* INCLUDED_Z_VIRT_H */
