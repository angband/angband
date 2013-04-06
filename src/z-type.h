#ifndef INCLUDED_ZTYPE_H
#define INCLUDED_ZTYPE_H

#include "h-basic.h"

typedef struct type_union type_union;

typedef enum type_val {
	T_END = 0,
	T_INT,
	T_FLOAT,
	T_CHAR,
	T_STRING
} type_val;

struct type_union
{
	type_val t; 
	union {
		float f;
		int i;
		char c;
		const char *s;
	} u;
};

extern type_union i2u(int i);
extern type_union f2u(float f);
extern type_union c2u(char c);
extern type_union s2u(const char *s);

static const type_union END = { T_END, { 0 } };

struct loc {
	int x;
	int y;
};

struct loc loc(int x, int y);

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
