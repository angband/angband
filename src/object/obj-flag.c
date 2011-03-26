/*
 * File: src/object/obj-flag.c
 * Purpose: functions to deal with object flags
 *
 * Copyright (c) 2011 Chris Carr
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
#include "angband.h"

/**
 * Details of the different object flags in the game.
 * See src/object/obj-flag.h for structure
 */
const struct object_flag object_flag_table[] =
{
    #define OF(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r) \
            { OF_##a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q,r },
    #include "list-object-flags.h"
    #undef OF
};

/**
 * Create a "mask" of flags of a specific type or ID threshold.
 *
 * \param f is the flag array we're filling
 * \param id is whether we're masking by ID level
 * \param ... is the list of flags or ID types we're looking for
 *
 * N.B. OFT_MAX must be the last item in the ... list
 */
void create_mask(bitflag *f, bool id, ...)
{
	const struct object_flag *of_ptr;
	int i;
	va_list args;

	of_wipe(f);

	va_start(args, id);

	/* Process each type in the va_args */
    for (i = va_arg(args, int); i != OFT_MAX; i = va_arg(args, int)) {
		for (of_ptr = object_flag_table; of_ptr->index < OF_MAX; of_ptr++)
			if ((id && of_ptr->id == i) || (!id && of_ptr->type == i))
				of_on(f, of_ptr->index);
	}

	va_end(args);

	return;
}

/**
 * Print a message when an object flag is identified by use.
 *
 * \param flag is the flag being noticed
 * \param name is the object name 
 */
void flag_message(int flag, char *name)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	if (!streq(of_ptr->message, ""))
		msg(of_ptr->message, name);

	return;
}

/**
 * Determine whether a flagset includes any curse flags.
 */
bool cursed_p(bitflag *f)
{
	bitflag f2[OF_SIZE];

	of_wipe(f2);
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);

	return of_is_inter(f, f2);
}

