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

unsigned int mem_flags = 0;

#define SZ(uptr)	*((size_t *)((char *)(uptr) - sizeof(size_t)))

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
	char *mem;

	/* Allow allocation of "zero bytes" */
	if (len == 0) return (NULL);

	mem = malloc(len + sizeof(size_t));
	if (!mem)
		quit("Out of Memory!");
	mem += sizeof(size_t);
	if (mem_flags & MEM_POISON_ALLOC)
		memset(mem, 0xCC, len);
	SZ(mem) = len;

	return mem;
}

void *mem_zalloc(size_t len)
{
	void *mem = mem_alloc(len);
	memset(mem, 0, len);
	return mem;
}

void mem_free(void *p)
{
	if (!p) return;

	if (mem_flags & MEM_POISON_FREE)
		memset(p, 0xCD, SZ(p));
	free((char *)p - sizeof(size_t));
}

void *mem_realloc(void *p, size_t len)
{
	char *m = p;

	/* Fail gracefully */
	if (len == 0) return (NULL);

	m = realloc(m ? m - sizeof(size_t) : NULL, len + sizeof(size_t));
	m += sizeof(size_t);

	/* Handle OOM */
	if (!m) quit("Out of Memory!");
	SZ(m) = len;

	return m;
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

void string_free(char *str)
{
	mem_free(str);
}

char *string_append(char *s1, const char *s2) {
	u32b len;
	if (!s1 && !s2) {
		return NULL;
	} else if (s1 && !s2) {
		return s1;
	} else if (!s1 && s2) {
		return string_make(s2);
	}
	len = strlen(s1);
	s1 = mem_realloc(s1, len + strlen(s2) + 1);
	strcpy(s1 + len, s2);
	return s1;
}
