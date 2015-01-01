/*
 * File: options.c
 * Purpose: Options table and definitions.
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
#include "option.h"
#include "ui-display.h"
#include "init.h"
#include "z-term.h"

typedef struct {
	const char *name;
	const char *description;
	int type;
	bool normal;
} option_entry;

/*
 * Option screen interface
 */
int option_page[OPT_PAGE_MAX][OPT_PAGE_PER] = { {0} };

static option_entry options[OPT_MAX] = {
#define OP(a, b, c, d)    { #a, b, OP_##c, d },
#include "list-options.h"
#undef OP
};

/* Accessor functions */
const char *option_name(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return options[opt].name;
}

const char *option_desc(int opt)
{
	if (opt >= OPT_MAX) return NULL;
	return options[opt].description;
}

int option_type(int opt)
{
	if (opt >= OPT_MAX)
		return 0;
	return options[opt].type;
}

#if 0 /* unused so far but may be useful in future */
static bool option_is_birth(int opt) { return (option_type(opt) == OP_BIRTH); }
static bool option_is_score(int opt) { return (option_type(opt) == OP_SCORE); }
#endif

static bool option_is_cheat(int opt) { return (option_type(opt) == OP_CHEAT); }

/* Setup functions */
bool option_set(const char *name, int val)
{
	size_t opt;

	/* Try normal options first */
	for (opt = 0; opt < OPT_MAX; opt++) {
		if (!options[opt].name || !streq(options[opt].name, name))
			continue;

		op_ptr->opt[opt] = val ? TRUE : FALSE;
		if (val && option_is_cheat(opt))
			op_ptr->opt[opt + 1] = TRUE;

		return TRUE;
	}

	return FALSE;
}

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

	/* Set defaults */
	for (opt = 0; opt < OPT_MAX; opt++)
		op_ptr->opt[opt] = options[opt].normal;

	/* 40ms for the delay factor */
	op_ptr->delay_factor = 40;

	/* 30% of HP */
	op_ptr->hitpoint_warn = 3;
}


/*
 * Write all current options to a user preference file.
 */
void option_dump(ang_file *f)
{
	int i, j;

	file_putf(f, "# Options\n\n");

	/* Dump window flags */
	for (i = 1; i < ANGBAND_TERM_MAX; i++)
	{
		/* Require a real window */
		if (!angband_term[i]) continue;

		/* Check each flag */
		for (j = 0; j < (int)N_ELEMENTS(window_flag_desc); j++)
		{
			/* Require a real flag */
			if (!window_flag_desc[j]) continue;

			/* Comment */
			file_putf(f, "# Window '%s', Flag '%s'\n",
				angband_term_name[i], window_flag_desc[j]);

			/* Dump the flag */
			if (window_flag[i] & (1L << j))
				file_putf(f, "window:%d:%d:1\n", i, j);
			else
				file_putf(f, "window:%d:%d:0\n", i, j);

			/* Skip a line */
			file_putf(f, "\n");
		}
	}
}

struct init_module options_module = {
	.name = "options",
	.init = init_options,
	.cleanup = NULL
};
