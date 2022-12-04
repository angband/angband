/**
 * \file z-queue.h
 * \brief Simple circular integer queue.
 *
 * Copyright (c) 2011 Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_Z_QUEUE_H
#define INCLUDED_Z_QUEUE_H

#include "h-basic.h"

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
