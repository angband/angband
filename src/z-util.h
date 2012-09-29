/* File z-util.h */

#ifndef INCLUDED_Z_UTIL_H
#define INCLUDED_Z_UTIL_H

#include "h-include.h"


/*
 * Extremely basic stuff, like global tmp and constant variables.
 * Also, some useful low level functions like streq() and such.
 * These can all be used as addressable functions.
 */



/**** Available variables ****/

/* Temporary Vars */
extern char char_tmp;
extern byte byte_tmp;
extern sint sint_tmp;
extern uint uint_tmp;
extern long long_tmp;
extern huge huge_tmp;
extern errr errr_tmp;

/* Temporary Pointers */
extern cptr cptr_tmp;
extern vptr vptr_tmp;


/* Constant pointers (NULL) */
extern cptr cptr_null;
extern vptr vptr_null;


/* A bizarre vptr that always points at itself */
extern vptr vptr_self;


/* A cptr to the name of the program */
extern cptr argv0;


/* Aux functions */
extern func_void plog_aux;
extern func_void quit_aux;
extern func_void core_aux;


/**** Available Functions ****/

/* Function that does nothing */
extern void func_nothing(void);

/* Functions that return basic "errr" codes */
extern errr func_success(void);
extern errr func_problem(void);
extern errr func_failure(void);

/* Functions that return bools */
extern bool func_true(void);
extern bool func_false(void);

/* Parse a long from a string, return chars used */
extern sint long_parse(long *lp, cptr str);


/* Test equality, prefix, suffix */
extern bool streq(cptr a, cptr b);
extern bool prefix(cptr big, cptr small);
extern bool suffix(cptr big, cptr small);

#ifndef HAS_STRICMP
extern int stricmp(cptr a, cptr b);
#endif

/* Print an error message */
extern void plog(cptr str);

/* Exit, perhaps with a message */
extern void quit(cptr str);

/* Dump core, with optional message */
extern void core(cptr str);



#endif

