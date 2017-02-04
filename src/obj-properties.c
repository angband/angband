/**
 * \file obj-properties.c
 * \brief functions to deal with object flags and modifiers
 *
 * Copyright (c) 2014 Chris Carr, Nick McConnell
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
#include "object.h"
#include "obj-gear.h"
#include "obj-pile.h"

struct obj_property *obj_properties;

/**
 * Details of the different object flags in the game.
 * See src/obj-properties.h for structure
 *
 * Note that sustain stat flags are included first, so that the index into 
 * the flag table for a sustain is the stat index + 1
 *
 * Note that any strings in the last position must have exactly one %s
 */
static const struct object_flag object_flag_table[] =
{
	{ OF_NONE, OFID_NONE, OFT_NONE, "NONE" },
	#define STAT(a, c, f, g, h, i)							\
		{ OF_##c, OFID_NORMAL, OFT_SUST, "Your %s glows." },
	#include "list-stats.h"
	#undef STAT
	#define OF(a, b, c, e, f) { OF_##a, b, c, f },
	#include "list-object-flags.h"
	#undef OF
};

/**
 * Object flag names
 */
static const char *flag_names[] =
{
	"NONE",
	#define STAT(a, c, f, g, h, i) #c,
	#include "list-stats.h"
	#undef STAT
	#define OF(a, b, c, e, f) #a,
	#include "list-object-flags.h"
	#undef OF
    ""
};

/**
 * Object modifier names
 */
static const char *mod_names[] =
{
	#define STAT(a, c, f, g, h, i) #a,
	#include "list-stats.h"
	#undef STAT
	#define OBJ_MOD(a, b) #a,
	#include "list-object-modifiers.h"
	#undef OBJ_MOD
	""
};

/**
 * Create a "mask" of object flags of a specific type or ID threshold.
 *
 * \param f is the flag array we're filling
 * \param id is whether we're masking by ID level
 * \param ... is the list of flags or ID types we're looking for
 *
 * N.B. OFT_MAX must be the last item in the ... list
 */
void create_obj_flag_mask(bitflag *f, bool id, ...)
{
	const struct object_flag *of;
	int i;
	va_list args;

	of_wipe(f);

	va_start(args, id);

	/* Process each type in the va_args */
    for (i = va_arg(args, int); i != OFT_MAX; i = va_arg(args, int)) {
		for (of = object_flag_table; of->index < OF_MAX; of++)
			if ((id && of->id == i) || (!id && of->type == i))
				of_on(f, of->index);
	}

	va_end(args);

	return;
}

/**
 * Return the name of a flag.
 */
const char *flag_name(int flag)
{
	return flag_names[flag];
}

/**
 * Return the index of a flag from its name.
 */
int flag_index_by_name(const char *name)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(flag_names); i++) {
		if (streq(name, flag_names[i])) {
			return i;
		}
	}

	return -1;
}

/**
 * Return the OFT_ type of a flag.
 */
int obj_flag_type(int flag)
{
	const struct object_flag *of = &object_flag_table[flag];

	return of->type;
}

/**
 * Print a message when an object flag is identified by use.
 *
 * \param flag is the flag being noticed
 * \param name is the object name 
 */
void flag_message(int flag, char *name)
{
	const struct object_flag *of = &object_flag_table[flag];

	if (!streq(of->message, ""))
		msg(of->message, name);

	return;
}

/**
 * Return the sustain flag of a given stat.
 */
int sustain_flag(int stat)
{
	if (stat < 0 || stat >= STAT_MAX) return -1;

	return object_flag_table[stat + 1].index;
}

/**
 * Return the name of a flag.
 */
const char *mod_name(int mod)
{
	return mod_names[mod];
}
