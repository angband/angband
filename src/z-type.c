#include "h-basic.h"
#include "z-form.h"
#include "z-term.h"
#include "ui.h"
#include "z-type.h"

#define TYPE_FUN(v2u, tv, T, v)	\
type_union v2u(T v) { 		\
	type_union r;			\
	r.u.v = v; 				\
	r.t = tv;				\
	return r;				\
}

TYPE_FUN(i2u, T_INT, int, i)
TYPE_FUN(c2u, T_CHAR, char, c)
TYPE_FUN(f2u, T_FLOAT, float, f)
TYPE_FUN(s2u, T_STRING, const char *, s)

struct point point(int x, int y) {
	struct point p;
	p.x = x;
	p.y = y;
	return p;
}
