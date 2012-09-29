/* File z-form.h */

#ifndef INCLUDED_Z_FORM_H
#define INCLUDED_Z_FORM_H

#include "h-include.h"

/*
 * Some useful functions dealing with varargs and formatting.
 * Most notable is a version of sprintf() which handles length.
 */


/*
 * Include the correct vararg's support
 */
#ifndef __MAKEDEPEND__
# include <stdarg.h>
#endif



/**** Available Functions ****/

/* Base Function -- Length based sprintf() */
extern uint vstrnfmt(char *buf, uint max, cptr fmt, va_list vp);

/* Base Function -- Do sprintf() into a static resizing buffer */
extern char *vformat(cptr fmt, va_list vp);

/* Length based sprintf(), with a max-length */
extern uint strnfmt(char *buf, uint max, cptr fmt, ...);

/* Length based sprintf(), without a max-length */
extern uint strfmt(char *buf, cptr fmt, ...);

/* Do sprintf() into a static resizing buffer */
extern char *format(cptr fmt, ...);

/* Vararg interface to plog() */
extern void plog_fmt(cptr fmt, ...);

/* Vararg interface to quit() */
extern void quit_fmt(cptr fmt, ...);

/* Vararg interface to core() */
extern void core_fmt(cptr fmt, ...);


#endif

