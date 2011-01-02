/*
 * File: slays.h
 * Purpose: List of slay types
 *
 * Copyright (c) 2007 Andrew Sidwell
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
#ifndef INCLUDED_SLAYS_H
#define INCLUDED_SLAYS_H

/* Types of slay (including brands) */
typedef enum
{
	#define SLAY(a, b, c, d, e, f, g, h, i, j)    SL_##x,
	#include "list-slays.h"
	#undef SLAY

	SL_MAX
} slay_type;

/*** Functions ***/


#endif /* INCLUDED_SLAYS_H */
