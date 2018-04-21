/**
   \file obj-desc.h
   \brief Create object name descriptions
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

/**
 * Modes for object_desc().
 */
enum {
	ODESC_BASE   = 0x00,   /*!< Only describe the base name */
	ODESC_COMBAT = 0x01,   /*!< Also show combat bonuses */
	ODESC_EXTRA  = 0x02,   /*!< Show charges/inscriptions/pvals */

	ODESC_FULL   = ODESC_COMBAT | ODESC_EXTRA,
	                       /*!< Show entire description */

	ODESC_STORE  = 0x04,   /*!< This is an in-store description */
	ODESC_PLURAL = 0x08,   /*!< Always pluralise */
	ODESC_SINGULAR    = 0x10,    /*!< Always singular */
	ODESC_SPOIL  = 0x20,    /*!< Display regardless of player knowledge */
	ODESC_PREFIX = 0x40,   /* */

	ODESC_CAPITAL = 0x80,	/*!< Capitalise object name */
	ODESC_TERSE = 0x100,  	/*!< Make terse names */
	ODESC_NOEGO = 0x200  	/*!< Don't show ego names */
};


extern const char *inscrip_text[];

void object_base_name(char *buf, size_t max, int tval, bool plural);
void object_kind_name(char *buf, size_t max, const struct object_kind *kind,
					  bool easy_know);
size_t obj_desc_name_format(char *buf, size_t max, size_t end, const char *fmt,
							const char *modstr, bool pluralise);
size_t object_desc(char *buf, size_t max, const struct object *obj, int mode);

#endif /* OBJECT_DESC_H */
