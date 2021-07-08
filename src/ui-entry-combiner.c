/**
 * \file ui-entry-combiner.h
 * \brief Define algorithms for combining property values
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
#include "ui-entry-combiner.h"
#include "z-virt.h"

struct combining_algorithm {
	const char *name;
	struct ui_entry_combiner_funcs funcs;
};


static void simple_combine_init(int v, int a,
	struct ui_entry_combiner_state *st);
static void dummy_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void dummy_combine_finish(struct ui_entry_combiner_state *st);
static void add_combine_accum_help(int x, int *accum);
static void add_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void add_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);
static void bitwise_or_combine_accum_help(int x, int *accum);
static void bitwise_or_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void bitwise_or_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);
static void first_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);
static void largest_combine_accum_help(int x, int *accum);
static void largest_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void largest_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);
static void last_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void last_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);
static void logical_combine_init(int v, int a,
	struct ui_entry_combiner_state *st);
static void logical_or_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void logical_or_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);
static void resist_0_combine_init(int v, int a,
	struct ui_entry_combiner_state *st);
static void resist_0_combine_accum_help(int x, int *posaccum, int *negaccum);
static void resist_0_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void resist_0_combine_finish(struct ui_entry_combiner_state *st);
static void resist_0_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);
static void smallest_combine_accum_help(int x, int *accum);
static void smallest_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st);
static void smallest_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux);


/* Sorted alphabetically by name to facilitate lookup. */
static struct combining_algorithm combiners[] = {
	{ "ADD", { simple_combine_init, add_combine_accum,
		dummy_combine_finish, add_vec } },
	{ "BITWISE_OR", { simple_combine_init, bitwise_or_combine_accum,
		dummy_combine_finish, bitwise_or_vec } },
	{ "FIRST", { simple_combine_init, dummy_combine_accum,
		dummy_combine_finish, first_vec } },
	{ "LARGEST", { simple_combine_init, largest_combine_accum,
		dummy_combine_finish, largest_vec } },
	{ "LAST", { simple_combine_init, last_combine_accum,
		dummy_combine_finish, last_vec } },
	{ "LOGICAL_OR", { logical_combine_init, logical_or_combine_accum,
		dummy_combine_finish, logical_or_vec } },
	{ "RESIST_0", { resist_0_combine_init, resist_0_combine_accum,
		resist_0_combine_finish, resist_0_vec } },
	{ "SMALLEST", { simple_combine_init, smallest_combine_accum,
		dummy_combine_finish, smallest_vec } }
};


/**
 * Return zero if the given name is not recognized.  Otherwise, return the
 * index to use ui_entry_combiner_get_funcs().
 */
int ui_entry_combiner_lookup(const char *name)
{
	/* Sorted alphabetically so use a binary search. */
	int ilow = 0, ihigh = N_ELEMENTS(combiners);

	while (1) {
		int imid, cmp;

		if (ilow == ihigh) {
			return 0;
		}
		imid = (ilow + ihigh) / 2;
		cmp = strcmp(combiners[imid].name, name);
		if (cmp == 0) {
			return imid + 1;
		}
		if (cmp < 0) {
			ilow = imid + 1;
		} else {
			ihigh = imid;
		}
	}
}


/**
 * Return zero and set *funcs to the functions for that combiner if ind is
 * valid.  Otherwise, return a nonzero value.
 */
int ui_entry_combiner_get_funcs(int ind, struct ui_entry_combiner_funcs *funcs)
{
	if (ind < 1 || ind > (int)N_ELEMENTS(combiners)) {
		return 1;
	}
	*funcs = combiners[ind - 1].funcs;
	return 0;
}


static void simple_combine_init(int v, int a,
	struct ui_entry_combiner_state *st)
{
	st->work = 0;
	st->accum = v;
	st->accum_aux = a;
}


static void dummy_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	/* Do nothing. */
}


static void dummy_combine_finish(struct ui_entry_combiner_state *st)
{
	/* Do nothing. */
}


static void add_combine_accum_help(int x, int *accum)
{
	if (x == UI_ENTRY_VALUE_NOT_PRESENT) {
		return;
	}
	if (x == UI_ENTRY_UNKNOWN_VALUE) {
		if (*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
			*accum = UI_ENTRY_UNKNOWN_VALUE;
		}
		return;
	}
	if (*accum == UI_ENTRY_UNKNOWN_VALUE ||
		*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
		*accum = x;
		return;
	}
	/*
	 * Just in case, guard against overflow or underflow.  Also guard
	 * adding up to the special values which are equal to INT_MAX and
	 * INT_MAX - 1.
	 */
	if (x > 0) {
		if (*accum <= INT_MAX - 2 - x) {
			*accum += x;
		} else {
			*accum = INT_MAX - 2;
		}
	} else if (x < 0) {
		if (*accum >= INT_MIN - x) {
			*accum += x;
		} else {
			*accum = INT_MIN;
		}
	}
}


static void add_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	add_combine_accum_help(v, &st->accum);
	add_combine_accum_help(a, &st->accum_aux);
}


static void add_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	int i;

	*accum = (n > 0) ? vals[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		add_combine_accum_help(vals[i], accum);
	}

	*accum_aux = (n > 0) ? auxs[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		add_combine_accum_help(auxs[i], accum_aux);
	}
}


static void bitwise_or_combine_accum_help(int x, int *accum)
{
	if (x == UI_ENTRY_VALUE_NOT_PRESENT) {
		return;
	}
	if (x == UI_ENTRY_UNKNOWN_VALUE) {
		if (*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
			*accum = UI_ENTRY_UNKNOWN_VALUE;
		}
		return;
	}
	if (*accum == UI_ENTRY_UNKNOWN_VALUE ||
		*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
		*accum = x;
	} else {
		*accum |= x;
	}
}


static void bitwise_or_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	bitwise_or_combine_accum_help(v, &st->accum);
	bitwise_or_combine_accum_help(a, &st->accum_aux);
}


static void bitwise_or_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	int i;

	*accum = (n > 0) ? vals[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		bitwise_or_combine_accum_help(vals[i], accum);
	}

	*accum_aux = (n > 0) ? auxs[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		bitwise_or_combine_accum_help(auxs[i], accum_aux);
	}
}


static void first_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	if (n > 0) {
		*accum = vals[0];
		*accum_aux = auxs[0];
	} else {
		*accum = UI_ENTRY_VALUE_NOT_PRESENT;
		*accum_aux = UI_ENTRY_VALUE_NOT_PRESENT;
	}
}


static void largest_combine_accum_help(int x, int *accum)
{
	if (x == UI_ENTRY_VALUE_NOT_PRESENT) {
		return;
	}
	if (x == UI_ENTRY_UNKNOWN_VALUE) {
		if (*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
			*accum = UI_ENTRY_UNKNOWN_VALUE;
		}
		return;
	}
	if (*accum == UI_ENTRY_UNKNOWN_VALUE ||
		*accum == UI_ENTRY_VALUE_NOT_PRESENT || *accum < x) {
		*accum = x;
	}
}


static void largest_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	largest_combine_accum_help(v, &st->accum);
	largest_combine_accum_help(a, &st->accum_aux);
}


static void largest_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	int i;

	*accum = (n > 0) ? vals[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		largest_combine_accum_help(vals[i], accum);
	}

	*accum_aux = (n > 0) ? auxs[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		largest_combine_accum_help(auxs[i], accum_aux);
	}
}


static void last_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	st->accum = v;
	st->accum_aux = a;
}


static void last_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	if (n > 0) {
		*accum = vals[n - 1];
		*accum_aux = auxs[n - 1];
	} else {
		*accum = UI_ENTRY_VALUE_NOT_PRESENT;
		*accum_aux = UI_ENTRY_VALUE_NOT_PRESENT;
	}
}


static void logical_combine_init(int v, int a,
	struct ui_entry_combiner_state *st)
{
	st->work = 0;
	if (v == UI_ENTRY_UNKNOWN_VALUE || v == UI_ENTRY_VALUE_NOT_PRESENT) {
		st->accum = v;
	} else {
		st->accum = (v != 0);
	}
	if (a == UI_ENTRY_UNKNOWN_VALUE || a == UI_ENTRY_VALUE_NOT_PRESENT) {
		st->accum_aux = a;
	} else {
		st->accum_aux = (a != 0);
	}
}


static void logical_or_combine_accum_help(int x, int *accum)
{
	if (x == UI_ENTRY_VALUE_NOT_PRESENT) {
		return;
	}
	if (x == UI_ENTRY_UNKNOWN_VALUE) {
		if (*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
			*accum = UI_ENTRY_UNKNOWN_VALUE;
		}
		return;
	}
	if (*accum == UI_ENTRY_UNKNOWN_VALUE ||
		*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
		*accum = (x != 0);
	} else {
		*accum = *accum || (x != 0);
	}
}


static void logical_or_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	logical_or_combine_accum_help(v, &st->accum);
	logical_or_combine_accum_help(a, &st->accum_aux);
}


static void logical_or_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	int i;

	*accum = UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 0; i < n; ++i) {
		logical_or_combine_accum_help(vals[i], accum);
	}

	*accum_aux = UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 0; i < n; ++i) {
		logical_or_combine_accum_help(auxs[i], accum_aux);
	}
}


static void resist_0_combine_init(int v, int a,
	struct ui_entry_combiner_state *st)
{
	/*
	 * Use the accum values in st for the most positive.  Allocate working
	 * space to store the most negative.
	 */
	int *work = mem_alloc(2 * sizeof(*work));

	st->work = work;
	if (v == UI_ENTRY_UNKNOWN_VALUE || v == UI_ENTRY_VALUE_NOT_PRESENT) {
		st->accum = v;
		work[0] = v;
	} else if (v > 0) {
		st->accum = v;
		work[0] = 0;
	} else {
		st->accum = 0;
		work[0] = v;
	}
	if (a == UI_ENTRY_UNKNOWN_VALUE || v == UI_ENTRY_VALUE_NOT_PRESENT) {
		st->accum_aux = a;
		work[1] = a;
	} else if (a > 0) {
		st->accum_aux = a;
		work[1] = 0;
	} else {
		st->accum_aux = 0;
		work[1] = a;
	}
}


static void resist_0_combine_accum_help(int x, int *pos, int *neg)
{
	if (x == UI_ENTRY_VALUE_NOT_PRESENT) {
		return;
	}
	if (x == UI_ENTRY_UNKNOWN_VALUE) {
		if (*pos == UI_ENTRY_VALUE_NOT_PRESENT) {
			*pos = UI_ENTRY_UNKNOWN_VALUE;
			*neg = UI_ENTRY_UNKNOWN_VALUE;
		}
		return;
	}
	if (x > 0) {
		if (*pos == UI_ENTRY_UNKNOWN_VALUE ||
			*pos == UI_ENTRY_VALUE_NOT_PRESENT) {
			*pos = x;
			*neg = 0;
		} else if (*pos < x) {
			*pos = x;
		}
	} else {
		if (*neg == UI_ENTRY_UNKNOWN_VALUE ||
			*neg == UI_ENTRY_VALUE_NOT_PRESENT) {
			*neg = x;
			*pos = 0;
		} else if (*neg > x) {
			*neg = x;
		}
	}
}


static void resist_0_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	int *work = st->work;

	resist_0_combine_accum_help(v, &st->accum, work);
	resist_0_combine_accum_help(a, &st->accum_aux, work + 1);
}


static void resist_0_combine_finish(struct ui_entry_combiner_state *st)
{
	int *work = st->work;

	if (work[0] < 0 && work[0] != UI_ENTRY_UNKNOWN_VALUE &&
		work[0] != UI_ENTRY_VALUE_NOT_PRESENT) {
		/* A vulnerability cancels a resist but not an immunity. */
		if (st->accum < 3 && st->accum != UI_ENTRY_UNKNOWN_VALUE &&
			st->accum != UI_ENTRY_VALUE_NOT_PRESENT) {
			--st->accum;
		}
	}
	if (work[1] < 0 && work[1] != UI_ENTRY_UNKNOWN_VALUE &&
		work[1] != UI_ENTRY_VALUE_NOT_PRESENT) {
		if (st->accum_aux < 3 &&
			st->accum_aux != UI_ENTRY_UNKNOWN_VALUE &&
			st->accum_aux != UI_ENTRY_VALUE_NOT_PRESENT) {
			--st->accum_aux;
		}
	}
	mem_free(work);
	st->work = 0;
}


static void resist_0_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	int i, neg;

	*accum = UI_ENTRY_UNKNOWN_VALUE;
	neg = UI_ENTRY_UNKNOWN_VALUE;
	for (i = 0; i < n; ++i) {
		resist_0_combine_accum_help(vals[i], accum, &neg);
	}
	if (neg < 0 && neg != UI_ENTRY_UNKNOWN_VALUE &&
		neg != UI_ENTRY_VALUE_NOT_PRESENT) {
		/* A vulnerability cancels a resist but not an immunity. */
		if (*accum < 3 && *accum != UI_ENTRY_UNKNOWN_VALUE &&
			*accum != UI_ENTRY_VALUE_NOT_PRESENT) {
			--*accum;
		}
	}

	*accum_aux = UI_ENTRY_UNKNOWN_VALUE;
	neg = UI_ENTRY_UNKNOWN_VALUE;
	for (i = 0; i < n; ++i) {
		resist_0_combine_accum_help(auxs[i], accum_aux, &neg);
	}
	if (neg < 0 && neg != UI_ENTRY_UNKNOWN_VALUE &&
		neg != UI_ENTRY_VALUE_NOT_PRESENT) {
		if (*accum_aux < 3 && *accum_aux != UI_ENTRY_UNKNOWN_VALUE &&
			*accum_aux != UI_ENTRY_VALUE_NOT_PRESENT) {
			--*accum_aux;
		}
	}
}


static void smallest_combine_accum_help(int x, int *accum)
{
	if (x == UI_ENTRY_VALUE_NOT_PRESENT) {
		return;
	}
	if (x == UI_ENTRY_UNKNOWN_VALUE) {
		if (*accum == UI_ENTRY_VALUE_NOT_PRESENT) {
			*accum = UI_ENTRY_UNKNOWN_VALUE;
		}
		return;
	}
	if (*accum == UI_ENTRY_UNKNOWN_VALUE ||
		*accum == UI_ENTRY_VALUE_NOT_PRESENT || *accum > x) {
		*accum = x;
	}
}


static void smallest_combine_accum(int v, int a,
	struct ui_entry_combiner_state *st)
{
	smallest_combine_accum_help(v, &st->accum);
	smallest_combine_accum_help(a, &st->accum_aux);
}


static void smallest_vec(int n, const int *vals, const int *auxs,
	int *accum, int *accum_aux)
{
	int i;

	*accum = (n > 0) ? vals[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		smallest_combine_accum_help(vals[i], accum);
	}

	*accum_aux = (n > 0) ? auxs[0] : UI_ENTRY_VALUE_NOT_PRESENT;
	for (i = 1; i < n; ++i) {
		smallest_combine_accum_help(auxs[i], accum_aux);
	}
}
