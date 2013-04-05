#ifndef INCLUDED_Z_QUEUE_H
#define INCLUDED_Z_QUEUE_H

#include <h-basic.h>

#if (!defined(HAVE_STDINT_H))
/* MSVC doesn't have stdint.h (which is C99), so we'll just
 * create the right pointer manually. */
#ifndef _UINTPTR_T_DEFINED
#ifdef UINT_PTR
typedef UINT_PTR uintptr_t;
#else
/* an integer type with enough bits to hold a pointer */
typedef unsigned long uintptr_t;
#endif
#endif
#endif

struct queue {
    uintptr_t *data;
    size_t size;
    int head;
    int tail;
};

struct queue *q_new(size_t size);
int q_len(struct queue *q);

void q_push(struct queue *q, uintptr_t item);
uintptr_t q_pop(struct queue *q);

void q_free(struct queue *q);

#define q_push_ptr(q, ptr) q_push((q), (uintptr_t)(ptr))
#define q_pop_ptr(q) ((void *)(q_pop((q))))

#define q_push_int(q, i) q_push((q), (uintptr_t)(i))
#define q_pop_int(q) ((int)(q_pop((q))))

#endif /* INCLUDED_Z_QUEUE_H */
