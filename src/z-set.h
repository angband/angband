/* z-set.h - sets of pointers */

#ifndef Z_SET_H
#define Z_SET_H

#include "h-basic.h"
#include <sys/types.h>

struct set;

extern struct set *set_new();
extern void set_free(struct set *s);
extern void set_add(struct set *s, void *p);
extern bool set_del(struct set *s, void *p);
extern size_t set_size(struct set *s);
extern void *set_choose(struct set *s);
extern void *set_get(struct set *s, size_t index);
extern void set_insert(struct set *s, size_t index, void *p);

#endif /* !Z_SET_H */
