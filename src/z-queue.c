/*
 * File: z-queue.c
 * Purpose: Simple circular integer queue.
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

struct queue *q_new(size_t size) {
    struct queue *q = (struct queue*)malloc(sizeof(struct queue));
    q->data = (uintptr_t*)malloc(sizeof(uintptr_t) * size);
    q->size = size;
    q->head = 0;
    q->tail = 0;
    return q;
}

int q_len(struct queue *q) {
    int len;
    if (q->tail >= q->head) {
        len = q->tail - q->head;
    } else {
        len = q->size - q->head + q->tail;
    }
    return len;
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
