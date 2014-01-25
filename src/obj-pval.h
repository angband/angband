/*
 * File: pval.h
 * Purpose: Structures and functions for dealing with object pvals
 *
 * Copyright (c) 2011 Chris Carr
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
#ifndef INCLUDED_PVAL_H
#define INCLUDED_PVAL_H

#include "angband.h"

/** Functions **/
bool object_add_pval(object_type *o_ptr, int pval, int flag);
bool object_this_pval_is_visible(const object_type *o_ptr, int pval);
int which_pval(const object_type *o_ptr, const int flag);
void object_pval_flags(const object_type *o_ptr, bitflag flags[MAX_PVALS][OF_SIZE]);
void object_pval_flags_known(const object_type *o_ptr, bitflag flags[MAX_PVALS][OF_SIZE]);

#endif /* !INCLUDED_PVAL_H */
