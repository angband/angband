/**
 * \file ui-entry-combiner.h
 * \brief Declare lookup and access to algorithms for combining property values
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
#ifndef INCLUDED_UI_ENTRY_COMBINER_H
#define INCLUDED_UI_ENTRY_COMBINER_H

#include "h-basic.h"

/*
 * This is the value to use in vals or aux_vals array when the real value is
 * unknown to the player.
 */
#define UI_ENTRY_UNKNOWN_VALUE (INT_MAX)

/*
 * This is the value to use in vals or aux_vals array when the value is to
 * be treated as not present.
 */
#define UI_ENTRY_VALUE_NOT_PRESENT (INT_MAX - 1)

struct ui_entry_combiner_state {
	void *work;
	int accum;
	int accum_aux;
};

struct ui_entry_combiner_funcs {
	/*
	 * There's two ways to use the combiner.  With one, you pass
	 * declare a struct ui_entry_combiner_state, pass it and the initial
	 * value and initial auxiliary value to *init_func, pass each
	 * subsequent value and auxiliary value along with the combiner state
	 * structure to *accum_func, and then call *finish_func to complete
	 * the process leaving the combined value in the accum field of the
	 * state structure and the combined auxiliary value in the accum_aux
	 * field of the state structure.
	 *
	 * The second way assumes you have the values and auxiliary values to
	 * be combined stored in arrays, then you can call *vec_func with those
	 * arrays to get the result.
	 *
	 * How auxiliary values are used and interpreted varies.  There's
	 * two current uses for them.  One is to store the time-dependent
	 * effect while the value stores the permanent effect.  The other is
	 * to store the sustain flag while the value holds the corresponding
	 * stat modifier.
	 *
	 * All the functions treat values equal to UI_ENTRY_UNKNOWN
	 * (value is not known to the player) or UI_ENTRY_VALUE_NOT_PRESENT
	 * (no value, for instance because of absent equipment) as special
	 * cases.
	 */
	void (*init_func)(int v, int a, struct ui_entry_combiner_state *st);
	void (*accum_func)(int v, int a, struct ui_entry_combiner_state *st);
	void (*finish_func)(struct ui_entry_combiner_state *st);
	void (*vec_func)(int n, const int *vals, const int *auxs, int *accum, int *accum_aux);
};

int ui_entry_combiner_lookup(const char *name);
int ui_entry_combiner_get_funcs(int ind, struct ui_entry_combiner_funcs *funcs);
#endif /* INCLUDED_UI_ENTRY_COMBINER_H */
