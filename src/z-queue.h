/**
 * \file z-queue.h
 * \brief Simple circular integer queue and integer priority queue.
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
    size_t head;
    size_t tail;
};

struct queue *q_new(size_t size);
bool q_resize(struct queue *q, size_t size);
size_t q_size(const struct queue *q);
size_t q_len(struct queue *q);

void q_push(struct queue *q, uintptr_t item);
uintptr_t q_pop(struct queue *q);

void q_free(struct queue *q);

#define q_push_ptr(q, ptr) q_push((q), (uintptr_t)(ptr))
#define q_pop_ptr(q) ((void *)(q_pop((q))))

#define q_push_int(q, i) q_push((q), (uintptr_t)(i))
#define q_pop_int(q) ((int)(q_pop((q))))


/*
 * Stores a priority queue where the priorities are integers, the lowest
 * value for the priority is popped first, and either an integer or pointer
 * can be stored along with the priority as a payload.  Storing both pointers
 * and integers within the same priority queue, however, is not supported.
 * The queue uses a min-heap packed into an array.
 */
struct priority_queue_element {
	union { void *p; int i; } payload;
	int priority;
};
struct priority_queue {
	struct priority_queue_element *data;
	size_t size, count;
};

struct priority_queue *qp_new(size_t size);
bool qp_resize(struct priority_queue *qp, size_t size,
		void (*payload_free)(void*));
void qp_flush(struct priority_queue *qp, void (*payload_free)(void*));
void qp_free(struct priority_queue *qp, void (*payload_free)(void*));

size_t qp_size(const struct priority_queue *qp);
size_t qp_len(const struct priority_queue *qp);

void qp_push_int(struct priority_queue *qp, int priority, int payload);
void qp_push_ptr(struct priority_queue *qp, int priority, void *payload);

int qp_pop_int(struct priority_queue *qp);
void *qp_pop_ptr(struct priority_queue *qp);

int qp_pushpop_int(struct priority_queue *qp,
		int priority, int payload);
void *qp_pushpop_ptr(struct priority_queue *qp,
		int priority, void *payload);

int qp_peek_int(const struct priority_queue *qp);
void *qp_peek_ptr(const struct priority_queue *qp);

bool qp_isinvalid(const struct priority_queue *qp);

/* Remove some function call overhead if assertions are stripped. */
#ifdef NDEBUG
#define qp_size(qp) ((qp)->size)
#define qp_len(qp) ((qp)->count)
#define qp_peek_int(qp) ((qp)->data[0].payload.i)
#define qp_peek_ptr(qp) ((qp)->data[0].payload.p)
#endif

#endif /* INCLUDED_Z_QUEUE_H */
