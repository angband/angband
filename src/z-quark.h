#ifndef INCLUDED_Z_QUARK_H
#define INCLUDED_Z_QUARK_H

#include "h-basic.h"

/* Quark type */
typedef size_t quark_t;


/* Return a quark for the string 'str' */
quark_t quark_add(const char *str);

/* Return the string corresponding to the quark */
const char *quark_str(quark_t q);

/* Initialise the quarks package */
errr quarks_init(void);

/* De-initialise the quarks package */
errr quarks_free(void);


#endif /* !INCLUDED_Z_QUARK_H */
