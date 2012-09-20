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
 * Display the script debug menu
 */
extern void do_cmd_script(void);

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

/*
 * Get the spell-index of the spell at position 'index' in
 * the spellbook described by 'o_ptr'.
 */
extern int get_spell_index(const object_type *o_ptr, int index);

/*
 * Get the name of the spell
 */
extern cptr get_spell_name(int tval, int index);

extern cptr get_spell_info(int tval, int index);

extern bool cast_spell(int tval, int index);

#endif /* INCLUDED_SCRIPT_H */

