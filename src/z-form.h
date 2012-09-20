/* File z-form.h */

/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_FORM_H
#define INCLUDED_Z_FORM_H

#include "h-basic.h"

/*
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

/* Format arguments into given bounded-length buffer */
extern uint vstrnfmt(char *buf, uint max, cptr fmt, va_list vp);

/* Simple interface to "vstrnfmt()" */
extern uint strnfmt(char *buf, uint max, cptr fmt, ...);

/* Simple interface to "vstrnfmt()", assuming infinite length */
extern uint strfmt(char *buf, cptr fmt, ...);

/* Format arguments into a static resizing buffer */
extern char *vformat(cptr fmt, va_list vp);

/* Simple interface to "vformat()" */
extern char *format(cptr fmt, ...);

/* Vararg interface to "plog()", using "format()" */
extern void plog_fmt(cptr fmt, ...);

/* Vararg interface to "quit()", using "format()" */
extern void quit_fmt(cptr fmt, ...);

/* Vararg interface to "core()", using "format()" */
extern void core_fmt(cptr fmt, ...);


#endif
