/*
 * File: obj-desc.h
 * Purpose: Create object name descriptions
 *
 * Copyright (c) 1997 - 2007 Angband contributors
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

#ifndef OBJECT_DESC_H
#define OBJECT_DESC_H

void object_base_name(char *buf, size_t max, int tval, bool plural);
void object_kind_name(char *buf, size_t max, const object_kind *kind, bool easy_know);
size_t obj_desc_name_format(char *buf, size_t max, size_t end, const char *fmt, const char *modstr, bool pluralise);
size_t object_desc(char *buf, size_t max, const object_type *o_ptr, int mode);

#endif /* OBJECT_DESC_H */
