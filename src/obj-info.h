/**
 * \file obj-info.h
 * \brief Object description code.
 *
 * Copyright (c) 2010 Andi Sidwell
 * Copyright (c) 2004 Robert Ruehlmann
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

#ifndef OBJECT_INFO_H
#define OBJECT_INFO_H

#include "z-textblock.h"

/**
 * Modes for object_info()
 */
typedef enum {
	OINFO_NONE   = 0x00, /* No options */
	OINFO_TERSE  = 0x01, /* Keep descriptions brief, e.g. for dumps */
	OINFO_SUBJ   = 0x02, /* Describe object from the character's POV */
	OINFO_EGO    = 0x04, /* Describe an ego template */
	OINFO_FAKE   = 0x08, /* Describe any template */
	OINFO_SPOIL  = 0x10, /* Description is for spoilers */
} oinfo_detail_t;


textblock *object_info(const struct object *obj, oinfo_detail_t mode);
textblock *object_info_ego(struct ego_item *ego);
void object_info_spoil(ang_file *f, const struct object *obj, int wrap);
void object_info_chardump(ang_file *f, const struct object *obj, int indent, int wrap);

#endif /* OBJECT_INFO_H */
