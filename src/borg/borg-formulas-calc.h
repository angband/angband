/**
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_BORG_FORMULAS_CALC_H
#define INCLUDED_BORG_FORMULAS_CALC_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG
#include "borg-trait.h"

/*
 * read a mathematic calculation from a dynamic formula.
 */
extern int parse_calculation_line(char *line, const char *full_line);

/*
 * Calculate a value from a dynamic formula read earlier
 *  using parse_calculation_line
 */
extern int32_t borg_calculate_dynamic(int formula, int range_index);

/*
 * free all the memory used by calculations
 */
extern void calculations_free(void);

#endif
#endif
