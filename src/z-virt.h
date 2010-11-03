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
	(T*)(mem_alloc((N) * sizeof(T)))

/* Allocate, and return, a thing of type T */
#define RNEW(T) \
	(T*)(mem_alloc(sizeof(T)))


/* Allocate, wipe, and return an array of type T[N] */
#define C_ZNEW(N, T) \
	(T*)(C_WIPE(C_RNEW(N, T), N, T))

/* Allocate, wipe, and return a thing of type T */
#define ZNEW(T) \
	(T*)(WIPE(RNEW(T), T))


/* Free one thing at P, return NULL */
#define FREE(P) (mem_free(P), P = NULL)

/* Replacements for malloc() and friends that die on failure. */
void *mem_alloc(size_t len);
void *mem_zalloc(size_t len);
void mem_free(void *p);
void *mem_realloc(void *p, size_t len);

char *string_make(const char *str);
void string_free(char *str);
char *string_append(char *s1, const char *s2);

enum {
	MEM_POISON_ALLOC = 0x00000001,
	MEM_POISON_FREE  = 0x00000002
};

extern unsigned int mem_flags;

#endif /* INCLUDED_Z_VIRT_H */
