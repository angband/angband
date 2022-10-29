/**
 * \file z-form.h
 * \brief Low-level text formatting (snprintf() replacement)
 *
 * Copyright (c) 1997 Ben Harrison
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
#ifndef INCLUDED_Z_FORM_H
#define INCLUDED_Z_FORM_H

#include "h-basic.h"

/**
 * This file provides functions very similar to "sprintf()", but which
 * not only parse some additional "format sequences", but also enforce
 * bounds checking, and allow repeated "appends" to the same buffer.
 *
 * See "z-form.c" for more detailed information about the routines,
 * including a list of the legal "format sequences".
 *
 * This file makes use of both "z-util.c" and "z-virt.c"
 */

/**** Available Functions ****/

/**
 * Format arguments into given bounded-length buffer
 */
extern size_t vstrnfmt(char *buf, size_t max, const char *fmt, va_list vp);

/**
 * Simple interface to "vstrnfmt()"
 */
extern size_t strnfmt(char *buf, size_t max, const char *fmt, ...)
	ATTRIBUTE ((format (printf, 3, 4)));

/**
 * Format arguments into a static resizing buffer
 */
extern char *vformat(const char *fmt, va_list vp);

/**
 * Free the memory allocated for the format buffer
 */
extern void vformat_kill(void);

/**
 * Append a formatted string to another string
 */
extern void strnfcat(char *str, size_t max, size_t *end, const char *fmt, ...)
	ATTRIBUTE ((format (printf, 4, 5)));

/**
 * Simple interface to "vformat()"
 */
extern char *format(const char *fmt, ...)
	ATTRIBUTE ((format (printf, 1, 2)));

/**
 * Vararg interface to "plog()", using "format()"
 */
extern void plog_fmt(const char *fmt, ...)
	ATTRIBUTE ((format (printf, 1, 2)));

/**
 * Vararg interface to "quit()", using "format()"
 */
extern void quit_fmt(const char *fmt, ...)
	ATTRIBUTE ((format (printf, 1, 2)));

#endif /* INCLUDED_Z_FORM_H */
