/**
 * \file option.h
 * \brief Options table and definitions.
 *
 * Copyright (c) 1997 Ben Harrison
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
#ifndef INCLUDED_OPTIONS_H
#define INCLUDED_OPTIONS_H

#include "z-file.h"

/**
 * Option types 
 */
enum
{
	OP_INTERFACE = 0,
	OP_BIRTH,
	OP_CHEAT,
	OP_SCORE,
	OP_SPECIAL,

	OP_MAX
};

/**
 * Option indexes 
 */
enum
{
    #define OP(a, b, c, d) OPT_##a,
    #include "list-options.h"
    #undef OP
	OPT_MAX
};


#define OPT(opt_name)	op_ptr->opt[OPT_##opt_name]

/**
 * Information for "do_cmd_options()".
 */
#define OPT_PAGE_MAX				OP_SCORE
#define OPT_PAGE_PER				20
#define OPT_PAGE_BIRTH				1

/**
 * The option data structures
 */
extern int option_page[OPT_PAGE_MAX][OPT_PAGE_PER];

/**
 * Functions 
*/
const char *option_name(int opt);
const char *option_desc(int opt);
int option_type(int opt);
bool option_set(const char *opt, int val);
void init_options(void);


#endif /* !INCLUDED_OPTIONS_H */
