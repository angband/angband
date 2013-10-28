/*
 * File: debug.c
 * Purpose: Simple debugging functions
 *
 * Copyright (c) 2007 Andrew Sidwell
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
#include "angband.h"
#include "debug.h"

typedef void debug_hook(const char *);

static void to_stderr(const char *out);

static debug_hook *d_out = to_stderr;

/*
 * Simple printing to stderr
 */
static void to_stderr(const char *out)
{
	fputs(out, stderr);
	fputs("\n", stderr);
}

/*
 * Output some text.
 *
 * Amongst other things, this should use the z-msg package so that ports can
 * display e.g. a debugging window, or send the output to file.
 */
void debug(const char *fmt, ...)
{
	va_list vp;
	char buffer[1024] = "";

	va_start(vp, fmt);
	vstrnfmt(buffer, sizeof(buffer), fmt, vp);
	va_end(vp);

	d_out(buffer);

	/* We are done */
	return;
}
