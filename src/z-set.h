/**
 * \file z-set.h
 * \brief Sets of pointers
 *
 * Copyright (c) 2013 elly+angband@leptoquark.net. See COPYING.
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


#ifndef Z_SET_H
#define Z_SET_H

#include "h-basic.h"
#include <sys/types.h>

struct set;

extern struct set *set_new(void);
extern void set_free(struct set *s);
extern void set_add(struct set *s, void *p);
extern bool set_del(struct set *s, void *p);
extern size_t set_size(struct set *s);
extern void *set_choose(struct set *s);
extern void *set_get(struct set *s, size_t index);
extern void set_insert(struct set *s, size_t index, void *p);

#endif /* !Z_SET_H */
