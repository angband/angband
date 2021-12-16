/**
 * \file stats/structs.h
 * \brief Data structures for lists not exported in headers elsewhere
 *
 * Copyright (c) 2011 Robert Au
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational,
 *    research,
 *    and not for profit purposes provided that this copyright and
 *    statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef STATS_STRUCTS_H
#define STATS_STRUCTS_H

#include "effects.h"
#include "monster.h"
#include "mon-spell.h"
#include "obj-slays.h"

/**
 * Entries for spell/activation descriptions
 */
typedef struct
{
	uint16_t index;      /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	const char *desc;    /* Effect description */
} info_entry;

#endif /* STATS_STRUCTS_H */
