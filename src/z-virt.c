/**
 * \file z-virt.c
 * \brief Memory management routines
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

/**
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
	/* Note: standard malloc(3) returns a non-null pointer if passed
	 * a length of 0. Not quite sure why Angband's wrapper has this
	 * behavior. */
	if (!len)
		return NULL;

	void *p = malloc(len);
	if (!p)
		quit("Out of memory!");
	return p;
}

void *mem_zalloc(size_t len)
{
	void *mem = mem_alloc(len);
	if (len)
		memset(mem, 0, len);
	return mem;
}

void mem_free(void *p)
{
	free(p);
}

void *mem_realloc(void *p, size_t len)
{
	/* Note: standard realloc(3) frees if passed a size of 0, so this
	 * wrapper has different behavior. */
	if (!len)
		return NULL;

	p = realloc(p, len);
	if (!p)
		quit("Out of Memory!");
	return p;
}

/**
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

char *string_append(char *s1, const char *s2)
{
	size_t len;
	if (!s1 && !s2) {
		return NULL;
	} else if (s1 && !s2) {
		return s1;
	} else if (!s1 && s2) {
		return string_make(s2);
	}
	len = strlen(s1);
	s1 = mem_realloc(s1, len + strlen(s2) + 1);
	my_strcpy(s1 + len, s2, strlen(s2) + 1);
	return s1;
}
