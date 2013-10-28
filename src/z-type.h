#ifndef INCLUDED_ZTYPE_H
#define INCLUDED_ZTYPE_H

typedef struct data_panel data_panel;
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

/* 
 * Helper classes for the display of typed data
*/

#define MAX_FMT		2
struct data_panel
{
	byte color;
	const char *label;
	const char *fmt;	/* printf format argument */
	type_union value[MAX_FMT];	/* (short) arugment list */
};

#endif /* !INCLUDED_ZTYPE_H */
