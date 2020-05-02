/**
 * \file option.c
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
#include "angband.h"
#include "init.h"
#include "option.h"

/**
 * Option screen interface
 */
int option_page[OPT_PAGE_MAX][OPT_PAGE_PER] = { {0} };

static struct option_entry {
	const char *name;
	const char *description;
	int type;
	bool normal;
} options[OPT_MAX] = {
	#define OP(a, b, c, d)    { #a, b, OP_##c, d },
	#include "list-options.h"
	#undef OP
};

/**
 * Given an option index, return its name
 */
const char *option_name(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return options[opt].name;
}

/**
 * Given an option index, return its description
 */
const char *option_desc(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return options[opt].description;
}

/**
 * Determine the type of option (score, birth etc)
 */
int option_type(int opt)
{
	if (opt >= OPT_MAX)
		return 0;
	return options[opt].type;
}

static bool option_is_cheat(int opt)
{
	return (option_type(opt) == OP_CHEAT);
}

/**
 * Set an option, return true if successful
 */
bool option_set(const char *name, int val)
{
	size_t opt;

	/* Try normal options first */
	for (opt = 0; opt < OPT_MAX; opt++) {
		if (!options[opt].name || !streq(options[opt].name, name))
			continue;

		player->opts.opt[opt] = val ? true : false;
		if (val && option_is_cheat(opt))
			player->opts.opt[opt + 1] = true;

		return true;
	}

	return false;
}

/**
 * Clear cheat options
 */
void options_init_cheat(void)
{
	int i;

	for (i = 0; i < OPT_MAX; i++) {
		if (option_is_cheat(i)) {
			player->opts.opt[i] = false;
			player->opts.opt[i + 1] = false;
		}
	}
}

/**
 * Set player default options
 */
void options_init_defaults(struct player_options *opts)
{
	/* Set defaults */
	int opt;
	for (opt = 0; opt < OPT_MAX; opt++)
		(*opts).opt[opt] = options[opt].normal;

	/* 40ms for the delay factor */
	(*opts).delay_factor = 40;

	/* 30% of HP */
	(*opts).hitpoint_warn = 3;
}

/**
 * Initialise options package
 */
void init_options(void)
{
	int opt, page;

	/* Allocate options to pages */
	for (page = 0; page < OPT_PAGE_MAX; page++) {
		int page_opts = 0;
		for (opt = 0; opt < OPT_MAX; opt++) {
			if ((options[opt].type == page) && (page_opts < OPT_PAGE_PER))
				option_page[page][page_opts++] = opt;
		}
		while (page_opts < OPT_PAGE_PER)
			option_page[page][page_opts++] = OPT_none;
	}
}

struct init_module options_module = {
	.name = "options",
	.init = init_options,
	.cleanup = NULL
};
