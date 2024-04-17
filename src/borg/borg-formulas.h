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

#ifndef INCLUDED_BORG_FORMULAS_H
#define INCLUDED_BORG_FORMULAS_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG
#include "borg-trait.h"

/* a simple growing array of pointers */
struct borg_array {
    int    max;
    int    count;
    void **items;
};

struct value_sec;

/* quick array stuff */
extern int borg_array_add(struct borg_array *a, void *item);

/* 
 * Note an error.
 * section is the section of the line the error is in, as found in borg.txt
 * full_line is the line as read from borg.txt
 * section_label is where in the parsing the system thinks it is
 * error for further error text to make the error clearer.
 */
extern void borg_formula_error(const char *section, const char *full_line, const char *section_label, const char * error);

/*
 * turn a "value(x, y)" into a number
 */
extern int32_t calculate_from_value(struct value_sec *value, int range_index);

/*
 * read a "value(x, y) and turn it into a structure
 */
extern struct value_sec *parse_value(char *line, char* full_line);

/*
 * Calculate the basic "power"
 */
extern int32_t borg_power_dynamic(void);

/*
 * Determine what level the borg is prepared to dive to.
 */
extern const char *borg_prepared_dynamic(int depth);

/*
 * Load the FORMULA SECTION from borg.txt
 */
extern bool borg_load_formulas(ang_file *fp);

/*
 * Free all memory used by formulas
 */
extern void borg_free_formulas(void);


#endif
#endif
