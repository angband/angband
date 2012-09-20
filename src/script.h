/* File: script.h */

#ifndef INCLUDED_SCRIPT_H
#define INCLUDED_SCRIPT_H

#include "angband.h"

/*
 * Initalize the scripting support
 */
extern errr script_init(void);

/*
 * Free the resources for the scripting support
 */
extern errr script_free(void);

/*
 * Execute a string of scripting code
 */
extern bool script_do_string(cptr script);

/*
 * Execute a file with scripting code
 */
extern bool script_do_file(cptr filename);

/*
 * Callback for using an object
 */
extern bool use_object(object_type *o_ptr, bool *ident);

#endif /* INCLUDED_SCRIPT_H */

