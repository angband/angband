/* File: script.h */

#ifndef INCLUDED_SCRIPT_H
#define INCLUDED_SCRIPT_H

#include "angband.h"

/* Object and spell useage.  To be put somewhere else eventually. */
/* Right now they're fine here.  -AS- */
extern bool use_object(object_type *o_ptr, bool *ident);
extern int get_spell_index(const object_type *o_ptr, int index);
extern cptr get_spell_name(int tval, int index);
extern void get_spell_info(int tval, int index, char *buf, size_t len);
extern bool cast_spell(int tval, int index);

#endif /* INCLUDED_SCRIPT_H */
