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
#include "z-util.h"

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

	/* Override with customized birth options. */
	options_restore_custom_birth(opts);

	/* 40ms for the delay factor */
	(*opts).delay_factor = 40;

	/* 30% of HP */
	(*opts).hitpoint_warn = 3;
}

/**
 * Record the birth options for later recall.
 *
 * Return true if successful.  Return false if the operation failed.
 */
bool options_save_custom_birth(struct player_options *opts)
{
	bool success = true;
	char path[1024];
	ang_file *f;

	path_build(path, sizeof(path), ANGBAND_DIR_USER,
		"customized_birth_options.txt");
	f = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (f) {
		int opt;

		if (!file_put(f, "# These are customized defaults for the birth options.\n")) {
			success = false;
		}
		if (!file_put(f, "# All lines begin with \"option:\" followed by the internal option name.\n")) {
			success = false;
		}
		if (!file_put(f, "# After the name is a colon followed by yes or no for the option's state.\n")) {
			success = false;
		}
		for (opt = 0; opt < OPT_MAX; opt++) {
			if (options[opt].type == OP_BIRTH) {
				if (!file_putf(f, "# %s\n",
						 options[opt].description)) {
					success = false;
				}
				if (!file_putf(f, "option:%s:%s\n",
						options[opt].name,
						((*opts).opt[opt]) ? "yes" : "no")) {
					success = false;
				}
			}
		}

		if (!file_close(f)) {
			success = false;
		}
	} else {
		success = false;
	}
	return success;
}

/**
 * Reset the birth options to the customized defaults.
 *
 * Return true if successful.  That includes the case where no customized
 * defaults are available.  When that happens, the birth options are reset
 * to the maintainer's defaults.  Return false if the customized defaults
 * are present but unreadable.
 */
bool options_restore_custom_birth(struct player_options *opts)
{
	bool success = true;
	char path[1024];

	path_build(path, sizeof(path), ANGBAND_DIR_USER,
		"customized_birth_options.txt");
	if (file_exists(path)) {
		/*
		 * Could use run_parser(), but that exits the application if
		 * there are syntax errors.  Therefore, use our own parsing.
		 */
		ang_file *f = file_open(path, MODE_READ, FTYPE_TEXT);

		if (f) {
			int linenum = 1;
			char buf[1024];

			while (file_getl(f, buf, sizeof(buf))) {
				int offset = 0;

				if (sscanf(buf, "option:%n%*s", &offset) == 0 &&
						offset > 0) {
					int opt = 0;

					while (1) {
						size_t lname;

						if (opt >= OPT_MAX) {
							msg("Unrecognized birth option at line %d of the customized birth options.", linenum);
							break;
						}
						if (!options[opt].name) {
							++opt;
							continue;
						}
						lname = strlen(
							options[opt].name);
						if (strncmp(options[opt].name,
							buf + offset, lname) == 0 &&
							buf[offset + lname] == ':') {
							if (strncmp("yes", buf + offset + lname + 1, 3) == 0 && contains_only_spaces(buf + offset + lname + 4)) {
								(*opts).opt[opt] = true;
							} else if (strncmp("no", buf + offset + lname + 1, 2) == 0 && contains_only_spaces(buf + offset + lname + 3)) {
								(*opts).opt[opt] = false;
							} else {
								msg("Value at line %d of the customized birth options is not yes or no.", linenum);
							}
							break;
						}
						++opt;
					}
				} else {
					offset = 0;
					if (sscanf(buf, "#%n*s", &offset) == 0 && offset == 0 && ! contains_only_spaces(buf)) {
						msg("Line %d of the customized birth options is not parseable.", linenum);
					}
				}
				++linenum;
			}
			if (!file_close(f)) {
				success = false;
			}
		} else {
			success = false;
		}
	} else {
		options_reset_birth(opts);
	}
	return success;
}

/**
 * Reset the birth options to the maintainer's defaults.
 */
void options_reset_birth(struct player_options *opts)
{
	int opt;
	for (opt = 0; opt < OPT_MAX; opt++)
		if (options[opt].type == OP_BIRTH) {
			(*opts).opt[opt] = options[opt].normal;
		}
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
