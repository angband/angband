/**
 * \file z-quark.h
 * \brief Save memory by storing strings in a global array, ensuring
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

#ifndef INCLUDED_Z_QUARK_H
#define INCLUDED_Z_QUARK_H

#include "h-basic.h"

/**
 * Quark type
 */
typedef size_t quark_t;


/**
 * Return a quark for the string 'str'
 */
quark_t quark_add(const char *str);

/**
 * Return the string corresponding to the quark
 */
const char *quark_str(quark_t q);

/**
 * Initialise the quarks package
 */
void quarks_init(void);

/**
 * De-initialise the quarks package
 */
void quarks_free(void);


#endif /* !INCLUDED_Z_QUARK_H */
