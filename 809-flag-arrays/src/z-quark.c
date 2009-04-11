/*
 * File: z-quark.c
 * Purpose: Save memory by storing strings in a global array, ensuring
 * that each is only allocated once.
 *
 * Copyright (c) 1997 Ben Harrison
 * Copyright (c) 2007 "Elly"
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
#include "z-quark.h"

static char **quarks;
static size_t nr_quarks = 1;
static size_t alloc_quarks = 0;

#define QUARKS_INIT	16

quark_t quark_add(const char *str)
{
	quark_t q;

	for (q = 1; q < nr_quarks; q++)
	{
		if (!strcmp(quarks[q], str))
			return q;
	}

	if (nr_quarks == alloc_quarks)
	{
		alloc_quarks *= 2;
		quarks = mem_realloc(quarks, alloc_quarks * sizeof(char *));
	}
	
	q = nr_quarks++;
	quarks[q] = string_make(str);

	return q;
}

const char *quark_str(quark_t q)
{
	return (q >= nr_quarks ? NULL : quarks[q]);
}

errr quarks_init(void)
{
	alloc_quarks = QUARKS_INIT;
	quarks = C_ZNEW(alloc_quarks, char *);

	return 0;
}

errr quarks_free(void)
{
	size_t i;

	for (i = 0; i < nr_quarks; i++)
		string_free(quarks[i]);

	FREE(quarks);
	return 0;
}
