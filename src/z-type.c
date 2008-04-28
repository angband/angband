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


void display_panel(const data_panel *panel, int count, bool left_adj, const region *bounds)
{
	int i;
	char buffer[50];
	int col = bounds->col;
	int row = bounds->row;
	int w = bounds->width;
	int offset = 0;

	region_erase(bounds);

	if (left_adj)
	{
		for (i = 0; i < count; i++)
		{
			int len = panel[i].label ? strlen(panel[i].label) : 0;
			if (offset < len) offset = len;
		}
		offset += 2;
	}

	for (i = 0; i < count; i++, row++)
	{
		int len;
		if (!panel[i].label) continue;
		Term_putstr(col, row, strlen(panel[i].label), TERM_WHITE, panel[i].label);

		strnfmt(buffer, sizeof(buffer), panel[i].fmt, panel[i].value[0], panel[i].value[1]);

		len = strlen(buffer);
		len = len < w - offset ? len : w - offset - 1;
		if (left_adj)
			Term_putstr(col+offset, row, len, panel[i].color, buffer);
		else
			Term_putstr(col+w-len, row, len, panel[i].color, buffer);
	}
}
