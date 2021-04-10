/**
 * \file main-spoil.c
 * \brief Support spoiler generation from the command line
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

#ifdef USE_SPOIL

#include "init.h"
#include "main.h"
#include "player-birth.h"
#include "wizard.h"

static struct {
	char letter;
	void (*func)(const char*);
	bool enabled;
	const char *path;
} opts[] = {
	{ 'a', spoil_artifact, false, NULL },
	{ 'm', spoil_mon_desc, false, NULL },
	{ 'M', spoil_mon_info, false, NULL },
	{ 'o', spoil_obj_desc, false, NULL },
};

const char help_spoil[] =
	"Spoiler generation mode, subopts\n"
	"              -a fname    Write artifact spoilers to fname\n"
	"              -m fname    Write brief monster spoilers to fname\n"
	"              -M fname    Write extended monster spoilers to fname\n"
	"              -o fname    Write object spoilers to fname";

/**
 * Usage:
 *
 * angband -mspoil -- [-a fname] [-m fname] [-M fname] [-o fname]
 *
 *   -a fname  Write artifact spoilers to a file named fname.
 *   -m fname  Write brief monster spoilers to a file named fname.
 *   -M fname  Write extended monster spoilers to a file named fname.
 *   -o fname  Write object spoilers to a file named fname.
 *
 * Bugs:
 * Would be nice to accept "-" as the file name and write the spoilers to
 * standard output in that case.  Given the current implementation in
 * wiz-spoil.c, that would require writing to a temporary file and then
 * copying the contents of that file to standard output.  Don't have temporary
 * file support in z-file.c so punting on that for now.
 *
 * The functions in wiz-spoil.c don't provide any feedback about whether the
 * operation failed so the exit code will always indicate success even if there
 * was a problem.
 */
errr init_spoil(int argc, char *argv[]) {
	/* Skip over argv[0] */
	int i = 1;
	int result = 0;

	/* Parse the arguments. */
	while (1) {
		bool badarg = false;
		int increment = 1;

		if (i >= argc) {
			break;
		}

		if (argv[i][0] == '-') {
			/* Try to match with a known option. */
			int j = 0;

			while (1) {
				if (j >= (int)N_ELEMENTS(opts)) {
					badarg = true;
					break;
				}

				if (argv[i][1] == opts[j].letter &&
					argv[i][2] == '\0') {
					if (i < argc - 1) {
						opts[j].enabled = true;
						/*
						 * Record the filename and
						 * skip parsing of it.
						 */
						opts[j].path = argv[i + 1];
						++increment;
					} else {
						printf("init-spoil: '%s' requires an argument, the name of the spoiler file\n", argv[i]);
						result = 1;
					}
					break;
				}

				++j;
			}
		} else {
			badarg = true;
		}

		if (badarg) {
			printf("init-spoil: bad argument '%s'\n", argv[i]);
			result = 1;
		}

		i += increment;
	}

	if (result == 0) {
		/* Generate the spoilers. */
		init_angband();
		player_generate(player, races, classes, false);

		for (i = 0; i < (int)N_ELEMENTS(opts); ++i) {
			if (!opts[i].enabled) continue;
			(*(opts[i].func))(opts[i].path);
		}

		cleanup_angband();
		exit(0);
	}

	return result;
}

#endif
