/**
 * \file z-queue.c
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

#include <stdlib.h>
#include "z-queue.h"
#include "z-virt.h"

/* Clear macro shortcuts for priority queue functions. */
#ifdef NDEBUG
#undef qp_size
#undef qp_len
#undef qp_peek_int
#undef qp_peek_ptr
#endif

struct queue *q_new(size_t size) {
	struct queue *q;

	if (size > SIZE_MAX / sizeof(uintptr_t) - 1) {
		return NULL;
	}
	q = (struct queue*)mem_alloc(sizeof(struct queue));
	if (!q) return NULL;
	q->data = (uintptr_t*)malloc(sizeof(uintptr_t) * (size + 1));
	q->size = size + 1;
	q->head = 0;
	q->tail = 0;
	return q;
}

bool q_resize(struct queue *q, size_t size) {
	if (size > SIZE_MAX / sizeof(uintptr_t) - 1) {
		return true;
	}
	assert(q->size > 0);
	if (size < q->size - 1 && q->head != q->tail
			&& (q->head > size || q->tail > size)) {
		uintptr_t *new_data = mem_alloc(sizeof(uintptr_t) * (size + 1));

		if (q->tail > q->head) {
			memcpy(new_data, q->data + q->head,
				MIN(q->tail - q->head, size + 1)
				* sizeof(uintptr_t));
			q->tail -= q->head;
		} else {
			size_t head_sz = MIN(q->size - q->head, size + 1);

			memcpy(new_data, q->data + q->head, head_sz
				* sizeof(uintptr_t));
			if (head_sz < size + 1) {
				memcpy(new_data + head_sz, q->data,
					(size + 1 - head_sz)
					* sizeof(uintptr_t));
			}
			q->tail = size;
		}
		q->head = 0;
		mem_free(q->data);
		q->data = new_data;
	} else {
		q->data = mem_realloc(q->data, sizeof(uintptr_t) * (size + 1));
	}
	q->size = size + 1;
	return false;
}

size_t q_size(const struct queue *q)
{
	assert(q->size > 0);
	return q->size - 1;
}

size_t q_len(struct queue *q) {
	return (q->tail >= q->head) ? q->tail - q->head :
		(q->size - q->head) + q->tail;
}

void q_push(struct queue *q, uintptr_t item) {
	q->data[q->tail] = item;
	q->tail = (q->tail + 1) % q->size;
	if (q->tail == q->head) abort();
}

uintptr_t q_pop(struct queue *q) {
	uintptr_t item = q->data[q->head];
	if (q->head == q->tail) abort();
	q->head = (q->head + 1) % q->size;
	return item;
}

void q_free(struct queue *q) {
	free(q->data);
	free(q);
}

/**
 * Help priority queue operations:  push the ith element of the heap upward
 * if its priority is less than that of its parent.
 */
static void up_heap(struct priority_queue *qp, size_t i)
{
	while (1) {
		size_t parent;
		struct priority_queue_element tmp;

		if (i == 0) {
			break;
		}
		parent = (i - 1) >> 1;
		if (qp->data[i].priority >= qp->data[parent].priority) {
			break;
		}
		tmp = qp->data[parent];
		qp->data[parent] = qp->data[i];
		qp->data[i] = tmp;
		i = parent;
	}
}

/**
 * Help priority queue operations:  push the ith element of the heap downward
 * if its priority is greater than that of its children.
 */
static void down_heap(struct priority_queue *qp, size_t i)
{
	while (1) {
		size_t child1 = (i << 1) + 1;
		struct priority_queue_element tmp;

		if (child1 >= qp->count) {
			break;
		}
		if (child1 == qp->count - 1) {
			/* There's only one child. */
			if (qp->data[i].priority <= qp->data[child1].priority) {
				break;
			}
			tmp = qp->data[i];
			qp->data[i] = qp->data[child1];
			qp->data[child1] = tmp;
			break;
		}
		if (qp->data[i].priority > qp->data[child1].priority) {
			if (qp->data[i].priority
					<= qp->data[child1 + 1].priority
					|| qp->data[child1].priority <
					qp->data[child1 + 1].priority) {
				tmp = qp->data[i];
				qp->data[i] = qp->data[child1];
				qp->data[child1] = tmp;
				i = child1;
			} else {
				tmp = qp->data[i];
				qp->data[i] = qp->data[child1 + 1];
				qp->data[child1 + 1] = tmp;
				i = child1 + 1;
			}
		} else if (qp->data[i].priority
				> qp->data[child1 + 1].priority) {
			tmp = qp->data[i];
			qp->data[i] = qp->data[child1 + 1];
			qp->data[child1 + 1] = tmp;
			i = child1 + 1;
		} else {
			break;
		}
	}
}

/**
 * Create a new integer priority queue.
 *
 * \param size is the maximum number of entries the queue can hold.
 * \return a pointer to the queue or NULL if it could not be created.
 *
 * Attempts to create priority queues with sizes larger than SIZE_MAX /
 * MAX(2, sizeof(struct priority_queue_element)) will fail.
 */
struct priority_queue *qp_new(size_t size)
{
	struct priority_queue *result;

	if (size > SIZE_MAX / MAX(2, sizeof(struct priority_queue_element))) {
		return NULL;
	}
	result = mem_alloc(sizeof(*result));
	result->data = mem_alloc(size * sizeof(*result->data));
	result->size = size;
	result->count = 0;
	return result;
}

/**
 * Resize an existing priority queue to hold up to size elements.
 *
 * \param qp is the priority queue to modify.
 * \param size is the new number of elements to hold.
 * \param payload_free will, if not NULL, be called on the pointer payload
 * of an existing element that does not fit in the new size before discarding
 * that element.
 * \return false if the operation was successful; otherwise return true.
 * If the resizing fails, the existing elements, length, and size of the
 * queue will remain unchanged.
 *
 * An attempt to resize a priority queue to have more than SIZE_MAX /
 * MAX(2, sizeof(struct priority_queue_element)) elements will fail.
 * Any existing elements that would not fit in the new size are discarded.
 */
bool qp_resize(struct priority_queue *qp, size_t size,
		void (*payload_free)(void*))
{
	assert(qp && qp->count <= qp->size);
	if (size > SIZE_MAX / MAX(2, sizeof(struct priority_queue_element))) {
		return true;
	}
	if (size < qp->count && payload_free) {
		size_t i;

		for (i = size; i < qp->count; ++i) {
			(*payload_free)(qp->data[i].payload.p);
		}
		qp->count = size;
	}
	qp->data = mem_realloc(qp->data, size * sizeof(*qp->data));
	qp->size = size;
	return false;
}

/**
 * Remove all entries currently in a priority queue.
 *
 * \param qp is queue to modify.
 * \param payload_free will, if not NULL, be called on the pointer payload
 * for all entries removed from the queue.
 */
void qp_flush(struct priority_queue *qp, void (*payload_free)(void*))
{
	assert(qp && qp->count <= qp->size);
	if (payload_free) {
		size_t i;

		for (i = 0; i < qp->count; ++i) {
			(*payload_free)(qp->data[i].payload.p);
		}
	}
	qp->count = 0;
}

/**
 * Release the resources allocated by a call to qp_new().
 *
 * \param qp is a priority queue returned by qp_new().  It may be NULL.
 * \param payload_free will, if not NULL, be called on the pointer payload
 * for any existing elements in the queue.
 */
void qp_free(struct priority_queue *qp, void (*payload_free)(void*))
{
	if (!qp) {
		return;
	}
	assert(qp->count <= qp->size);
	if (payload_free) {
		size_t i;

		for (i = 0; i < qp->count; ++i) {
			(*payload_free)(qp->data[i].payload.p);
		}
	}
	mem_free(qp->data);
	mem_free(qp);
}

/**
 * Return the maximum number of elements a priority queue can hold.
 */
size_t qp_size(const struct priority_queue *qp)
{
	assert(qp && qp->count <= qp->size);
	return qp->size;
}

/**
 * Return the number of elements currently in a priority queue.
 */
size_t qp_len(const struct priority_queue *qp)
{
	assert(qp && qp->count <= qp->size);
	return qp->count;
}

/**
 * Push an integer, payload, with the given priority on to a priority queue.
 *
 * \param qp is the priority queue to modify.
 * \param priority is the priority for the new element.
 * \param payload is the integer payload for the new element.
 *
 * Has undefined behavior if the priority queue is already full or if the
 * queue has existing elements and some of those were pushed onto the queue
 * with pointer payloads.
 */
void qp_push_int(struct priority_queue *qp, int priority, int payload)
{
	assert(qp && qp->count < qp->size);
	qp->data[qp->count].priority = priority;
	qp->data[qp->count].payload.i = payload;
	up_heap(qp, qp->count);
	++qp->count;
}

/**
 * Push a pointer, payload, with the given priority on to a priority queue.
 *
 * \param qp is the priority queue to modify.
 * \param priority is the priority for the new element.
 * \param payload is the pointer payload for the new element.
 *
 * Has undefined behavior if the priority queue is already full or if the
 * queue has existing elements and some of those were pushed onto the queue
 * with integer payloads.
 */
void qp_push_ptr(struct priority_queue *qp, int priority, void *payload)
{
	assert(qp && qp->count < qp->size);
	qp->data[qp->count].priority = priority;
	qp->data[qp->count].payload.p = payload;
	up_heap(qp, qp->count);
	++qp->count;
}

/**
 * Pop the element at the head of a priority queue and return its integer
 * payload.
 *
 * \param qp is the priority queue to modify.
 * \return the integer payload for the element that was at the head of the
 * queue.
 *
 * Has undefined behavior if there are no elements currently in the queue
 * or if the queue has one or more elements that were added with pointer
 * payloads.
 */
int qp_pop_int(struct priority_queue *qp)
{
	int result;

	assert(qp && qp->count > 0 && qp->count <= qp->size);
	result = qp->data[0].payload.i;
	--qp->count;
	qp->data[0] = qp->data[qp->count];
	down_heap(qp, 0);
	return result;
}

/**
 * Pop the element at the head of a priority queue and return its pointer
 * payload.
 *
 * \param qp is the priority queue to modify.
 * \return the pointer payload for the element that was at the head of the
 * queue.
 *
 * Has undefined behavior if there are no elements currently in the queue
 * or if the queue has one or more elements that were added with integer
 * payloads.
 */
void *qp_pop_ptr(struct priority_queue *qp)
{
	void *result;

	assert(qp && qp->count > 0 && qp->count <= qp->size);
	result = qp->data[0].payload.p;
	--qp->count;
	qp->data[0] = qp->data[qp->count];
	down_heap(qp, 0);
	return result;
}

/**
 * Push an integer, payload, with the given priority on to a priority queue,
 * pop the element at the head of the queue, and return that popped element's
 * integer payload.
 *
 * \param qp is the priority queue to modify
 * \param priority is the priority for the new element.
 * \param payload is the integer payload for the new element.
 * \return the integer payload for the element that was at the head of the
 * queue after pushing the element given by priority and payload.
 *
 * Has better performance than calling qp_push_int(qp, priority, payload) and
 * then qp_pop_int(qp).
 * Has undefined behavior if the priority queue has existing elements and
 * some of those were pushed onto the queue with pointer payloads.
 */
int qp_pushpop_int(struct priority_queue *qp, int priority, int payload)
{
	int result;

	assert(qp && qp->count <= qp->size);
	if (qp->count == 0 || priority <= qp->data[0].priority) {
		result = payload;
	} else {
		result = qp->data[0].payload.i;
		if (priority <= qp->data[qp->count - 1].priority) {
			qp->data[0].priority = priority;
			qp->data[0].payload.i = payload;
		} else {
			qp->data[0] = qp->data[qp->count - 1];
			qp->data[qp->count - 1].priority = priority;
			qp->data[qp->count - 1].payload.i = payload;
		}
		down_heap(qp, 0);
	}

	return result;
}

/**
 * Push a pointer, payload, with the given priority on to a priority queue,
 * pop the element at the head of the queue, and return that popped element's
 * pointer payload.
 *
 * \param qp is the priority queue to modify
 * \param priority is the priority for the new element.
 * \param payload is the pointer payload for the new element.
 * \return the pointer payload for the element that was at the head of the
 * queue after pushing the element given by priority and payload.
 *
 * Has better performance than calling qp_push_ptr(qp, priority, payload) and
 * then qp_pop_ptr(qp).
 * Has undefined behavior if the priority queue has existing elements and
 * some of those were pushed onto the queue with integer payloads.
 */
void *qp_pushpop_ptr(struct priority_queue *qp, int priority, void *payload)
{
	void *result;

	assert(qp && qp->count <= qp->size);
	if (qp->count == 0 || priority <= qp->data[0].priority) {
		result = payload;
	} else {
		result = qp->data[0].payload.p;
		if (priority <= qp->data[qp->count - 1].priority) {
			qp->data[0].priority = priority;
			qp->data[0].payload.p = payload;
		} else {
			qp->data[0] = qp->data[qp->count - 1];
			qp->data[qp->count - 1].priority = priority;
			qp->data[qp->count - 1].payload.p = payload;
		}
		down_heap(qp, 0);
	}

	return result;
}

/**
 * Return the integer payload for the element at the head of a priority queue.
 *
 * Has undefined behavior if the priority queue is empty or has one or more
 * existing elements that were pushed with pointer payloads.
 */
int qp_peek_int(const struct priority_queue *qp)
{
	assert(qp && qp->count > 0 && qp->count <= qp->size);
	return qp->data[0].payload.i;
}

/**
 * Return the pointer payload for the element at the head of a priority queue.
 *
 * Has undefined behavior if the priority queue is empty or has one or more
 * existing elements that were pushed with integer payloads.
 */
void *qp_peek_ptr(const struct priority_queue *qp)
{
	assert(qp && qp->count > 0 && qp->count <= qp->size);
	return qp->data[0].payload.p;
}

/**
 * Return true if a priority queue does not satisfy the expected invariants;
 * otherwise return false.
 */
bool qp_isinvalid(const struct priority_queue *qp)
{
	size_t start, parent;

	if (!qp || qp->count > qp->size) {
		return true;
	}

	/* Queues with less than two elements have nothing to check. */
	if (qp->count < 2) {
		return false;
	}

	if ((qp->count & 1) == 0) {
		/*
		 * On the deepest layer, there is a node that does not have
		 * a sibling with the same parent.
		 */
		parent = (qp->count - 2) >> 1;
		if (qp->data[qp->count - 1].priority
				< qp->data[parent].priority) {
			return true;
		}
		start = qp->count - 2;
	} else {
		/* All nodes on the deepest layer have a sibling. */
		start = qp->count - 1;
	}

	while (start > 1) {
		parent = (start - 1) >> 1;
		if (qp->data[start].priority < qp->data[parent].priority
				|| qp->data[start - 1].priority
				< qp->data[parent].priority) {
			return true;
		}
		start -= 2;
	}
	return false;
}
