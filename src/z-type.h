#ifndef INCLUDED_ZTYPE_H
#define INCLUDED_ZTYPE_H

#include "h-basic.h"

struct loc {
	int x;
	int y;
};

struct loc loc(int x, int y);


/**
 * Defines a (value, name) pairing.  Variable names used are historical.
 */
typedef struct grouper grouper;
struct grouper {
	int tval;
	const char *name;
};

/*
 * A set of points that can be constructed to apply a set of changes to
 */
struct point_set {
	int n;
	int allocated;
	struct loc *pts;
};

struct point_set *point_set_new(int initial_size);
void point_set_dispose(struct point_set *ps);
void add_to_point_set(struct point_set *ps, int y, int x);
int point_set_size(struct point_set *ps);
int point_set_contains(struct point_set *ps, int y, int x);

#endif /* !INCLUDED_ZTYPE_H */
