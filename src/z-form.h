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

/* MSVC doesn't have va_copy (which is C99) or an alternative, so we'll just
 * copy the SRC pointer. In other cases we'll use va_copy() as we should. */
#ifdef _MSC_VER
#define VA_COPY(DST, SRC) (DST) = (SRC)
#else
#define VA_COPY(DST, SRC) va_copy(DST, SRC)
#endif


/**** Available Functions ****/

/* Format arguments into given bounded-length buffer */
extern size_t vstrnfmt(char *buf, size_t max, const char *fmt, va_list vp);

/* Simple interface to "vstrnfmt()" */
extern size_t strnfmt(char *buf, size_t max, const char *fmt, ...);

/* Format arguments into a static resizing buffer */
extern char *vformat(const char *fmt, va_list vp);

/* Free the memory allocated for the format buffer */
extern void vformat_kill(void);

/* Append a formatted string to another string */
extern void strnfcat(char *str, size_t max, size_t *end, const char *fmt, ...);

/* Simple interface to "vformat()" */
extern char *format(const char *fmt, ...);

/* Vararg interface to "plog()", using "format()" */
extern void plog_fmt(const char *fmt, ...);

/* Vararg interface to "quit()", using "format()" */
extern void quit_fmt(const char *fmt, ...);


#endif /* INCLUDED_Z_FORM_H */
