/**
 * \file z-virt.h
 * \brief Memory management
 *
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_VIRT_H
#define INCLUDED_Z_VIRT_H

#include "h-basic.h"


/**
 * Replacements for malloc() and friends that die on failure.
 */
void *mem_alloc(size_t len);
void *mem_zalloc(size_t len);
void mem_free(void *p);
void *mem_realloc(void *p, size_t len);

/**
 * On NDS, we might need to allocate some data into external memory
 * with additional restrictions (no 8-bit writes). These "alt" methods
 * provide this; on other platforms, they are aliased.
 */
#if defined(NDS) && !defined(__3DS__)
void *mem_alloc_alt(size_t len);
void *mem_zalloc_alt(size_t len);
void mem_free_alt(void *p);
void *mem_realloc_alt(void *p, size_t len);
bool mem_is_alt_alloc(void *p);
#else
#define mem_alloc_alt mem_alloc
#define mem_zalloc_alt mem_zalloc
#define mem_free_alt mem_free
#define mem_realloc_alt mem_realloc
#define mem_is_alt_alloc(p) (false)
#endif

char *string_make(const char *str);
void string_free(char *str);
char *string_append(char *s1, const char *s2);

#endif /* INCLUDED_Z_VIRT_H */
