/*
 * File: z-virt.c
 * Purpose: Memory management routines
 *
 * Copyright (c) 1997 Ben Harrison.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "z-virt.h"
#include "z-util.h"

/*
 * Hooks for platform-specific memory allocation.
 */
static mem_alloc_hook ralloc_aux;
static mem_free_hook rnfree_aux;
static mem_realloc_hook realloc_aux;


/*
 * Set the hooks for the memory system.
 */
bool mem_set_hooks(mem_alloc_hook alloc, mem_free_hook free, mem_realloc_hook realloc)
{
	/* Error-check */
	if (!alloc || !free || !realloc) return FALSE;

	/* Set up hooks */
	ralloc_aux = alloc;
	rnfree_aux = free;
	realloc_aux = realloc;

	return TRUE;
}


/*
 * Allocate `len` bytes of memory.
 *
 * Returns:
 *  - NULL if `len` == 0; or
 *  - a pointer to a block of memory of at least `len` bytes
 *
 * Doesn't return on out of memory.
 */
void *mem_alloc(size_t len)
{
	void *mem;

	/* Allow allocation of "zero bytes" */
	if (len == 0) return (NULL);

	/* Allocate some memory */
	if (ralloc_aux) mem = (*ralloc_aux)(len);
	else            mem = malloc(len);

	/* Handle OOM */
	if (!mem) quit("Out of Memory!");

	return mem;
}


/*
 * Free the memory pointed to by `p`.
 *
 * Returns NULL.
 */
void *mem_free(void *p)
{
	/* Easy to free nothing */
	if (!p) return (NULL);

	/* Free memory */
	if (rnfree_aux) (*rnfree_aux)(p);
	else            free(p);

	/* Done */
	return (NULL);
}


/*
 * Allocate `len` bytes of memory, copying whatever is in `p` with it.
 *
 * Returns:
 *  - NULL if `len` == 0 or `p` is NULL; or
 *  - a pointer to a block of memory of at least `len` bytes
 *
 * Doesn't return on out of memory.
 */
void *mem_realloc(void *p, size_t len)
{
	void *mem;

	/* Fail gracefully */
	if (!p || len == 0) return (NULL);

	if (realloc_aux) mem = (*realloc_aux)(p, len);
	else             mem = realloc(p, len);

	/* Handle OOM */
	if (!mem) quit("Out of Memory!");

	return mem;
}



/*
 * Duplicates an existing string `str`, allocating as much memory as necessary.
 */
char *string_make(const char *str)
{
	char *res;
	size_t siz;

	/* Error-checking */
	if (!str) return NULL;

	/* Allocate space for the string (including terminator) */
	siz = strlen(str) + 1;
	res = mem_alloc(siz);

	/* Copy the string (with terminator) */
	my_strcpy(res, str, siz);

	return res;
}


/*
 * Un-allocate a string allocated above.
 */
#undef string_free
char *string_free(char *str)
{
	/* Kill the buffer of chars we must have allocated above */
	return mem_free(str);
}
