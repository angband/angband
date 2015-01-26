/**
 * \file debug.h
 * \brief Simple debugging functions
 *
 * Copyright (c) 2007 Andi Sidwell
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
#ifndef INCLUDED_DEBUG_H
#define INCLUDED_DEBUG_H

/**
 * Send the formatted string to whatever debug console or output is set up.
 *
 * The output will be treated logically as a single line, so do not include
 * newline characters in the format string.
 *
 * It is recommended you call debug() with "DHERE" at the beginning of your
 * format string, like so:
 * 	debug(DHERE "important info");
 *
 * This gives you file and line number information.
 */
void debug(const char *fmt, ...);

#endif /* INCLUDED_DEBUG_H */
