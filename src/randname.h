/**
 * \file randname.h
 * \brief Random name generation
 *
 * Copyright (c) 2007 Antony Sidwell, Sheldon Simms
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
#ifndef RANDNAME_H
#define RANDNAME_H

/**
 * The different types of name randname.c can generate
 * which is also the number of sections in names.txt
 */
typedef enum {
	RANDNAME_TOLKIEN = 1,
	RANDNAME_SCROLL,
 
	/* End of type marker - not a valid name type */
	RANDNAME_NUM_TYPES 
} randname_type;

extern const char *** name_sections;

/**
 * Make a random name.
 */
extern size_t randname_make(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen, const char ***wordlist);

#endif /* RANDNAME_H */
