/* z-set.c - sets of pointers */

#include "z-set.h"

#include "z-rand.h"
#include "z-virt.h"

struct set {
	void **elems;
	size_t allocated;
	size_t filled;
};

static void _set_check(struct set *s) {
	assert(s->allocated >= s->filled);
	assert(!s->allocated || s->elems);
}

static void _set_grow(struct set *s) {
	size_t nsz = s->allocated ? s->allocated * 2 : 16;
	s->elems = mem_realloc(s->elems, nsz * sizeof(void*));
	memset(s->elems + s->allocated, 0, sizeof(void*) * (nsz - s->allocated));
	s->allocated = nsz;
}

static int _set_find(struct set *s, void *p) {
	size_t i;
	for (i = 0; i < s->filled; i++)
		if (s->elems[i] == p)
			return i;
	return -1;
}

struct set *set_new(void) {
	struct set *s = mem_zalloc(sizeof *s);
	return s;
}

void set_free(struct set *s) {
	_set_check(s);
	mem_free(s->elems);
	mem_free(s);
}

void set_add(struct set *s, void *p) {
	_set_check(s);
	if (s->allocated == s->filled)
		_set_grow(s);
	s->elems[s->filled++] = p;
}

bool set_del(struct set *s, void *p) {
	ssize_t i;
	_set_check(s);

	i = _set_find(s, p);
	if (i < 0)
		return FALSE;
	/* overwrite elem i with the last elem, drop the size of the set
	 * if the elem to delete is the last elem, this is a noop */
	s->elems[i] = s->elems[--s->filled];
	return TRUE;
}

size_t set_size(struct set *s) {
	return s->filled;
}

void *set_choose(struct set *s) {
	_set_check(s);
	if (!s->filled)
		return NULL;
	return s->elems[randint0(s->filled)];
}

void *set_get(struct set *s, size_t index) {
	_set_check(s);
	if (index >= s->filled)
		return NULL;
	return s->elems[index];
}

void set_insert(struct set *s, size_t index, void *p) {
	while (index >= s->allocated)
		_set_grow(s);
	s->elems[index] = p;
	if (index >= s->filled)
		s->filled = index + 1;
}
