/**
 * \file save-charoutput.c
 * \brief Write short human-readable character synopsis for angband.live
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

#include "init.h"
#include "player.h"
#include "z-file.h"

/* Based on the adaptation of Exo's patch to frogcomposband. */
bool save_charoutput(void)
{
	char path[1024];
	ang_file *fo;
	bool written = true;

	path_build(path, sizeof(path), ANGBAND_DIR_USER, "CharOutput.txt");
	fo = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (fo) {
		if (! file_put(fo, "{\n")) written = false;
		if (! file_putf(fo, "race: \"%s\",\n", player->race->name)) written = false;
		if (! file_putf(fo, "class: \"%s\",\n", player->class->name)) written = false;
		if (! file_put(fo, "mapName: \"Angband\",\n")) written = false;
		if (! file_putf(fo, "dLvl: \"%i\",\n", player->depth)) written = false;
		if (! file_putf(fo, "cLvl: \"%i\",\n", player->lev)) written = false;
		if (! file_putf(fo, "isDead: \"%i\",\n", (player->is_dead) ? 1 : 0)) written = false;
		if (! file_putf(fo, "killedBy: \"%s\"\n", player->died_from)) written = false;
		if (! file_put(fo, "}")) written = false;
		if (! file_close(fo)) written = false;
	} else {
		written = false;
	}
	return written;
}

