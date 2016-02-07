/**
 * \file score.c
 * \brief Highscore handling for Angband
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "buildid.h"
#include "game-world.h"
#include "init.h"
#include "score.h"


/**
 * Calculates the total number of points earned (wow - NRM)
 */
static long total_points(void)
{
	return (player->max_exp + (100 * player->max_depth));
}


/**
 * Read in a highscore file.
 */
size_t highscore_read(high_score scores[], size_t sz)
{
	char fname[1024];
	ang_file *scorefile;
	size_t i;

	/* Wipe current scores */
	memset(scores, 0, sz * sizeof(high_score));

	path_build(fname, sizeof(fname), ANGBAND_DIR_SCORES, "scores.raw");
	scorefile = file_open(fname, MODE_READ, FTYPE_TEXT);

	if (!scorefile) return true;

	for (i = 0; i < sz; i++)
		if (file_read(scorefile, (char *)&scores[i], sizeof(high_score)) <= 0)
			break;

	file_close(scorefile);

	return i;
}


/**
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
size_t highscore_where(const high_score *entry, const high_score scores[],
					   size_t sz)
{
	size_t i;

	/* Read until we get to a higher score */
	for (i = 0; i < sz; i++) {
		long entry_pts = strtoul(entry->pts, NULL, 0);
		long score_pts = strtoul(scores[i].pts, NULL, 0);

		if (entry_pts >= score_pts)
			return i;

		if (scores[i].what[0] == '\0')
			return i;
	}

	/* The last entry is always usable */
	return sz - 1;
}

size_t highscore_add(const high_score *entry, high_score scores[], size_t sz)
{
	size_t slot = highscore_where(entry, scores, sz);

	memmove(&scores[slot + 1], &scores[slot],
			sizeof(high_score) * (sz - 1 - slot));
	memcpy(&scores[slot], entry, sizeof(high_score));

	return slot;
}

static size_t highscore_count(const high_score scores[], size_t sz)
{
	size_t i;
	for (i = 0; i < sz; i++)
		if (scores[i].what[0] == '\0')
			break;

	return i;
}


/**
 * Actually place an entry into the high score file
 * Return the location (0 is best) or -1 on "failure"
 */
static void highscore_write(const high_score scores[], size_t sz)
{
	size_t n;

	ang_file *lok;
	ang_file *scorefile;

	char old_name[1024];
	char cur_name[1024];
	char new_name[1024];
	char lok_name[1024];

	path_build(old_name, sizeof(old_name), ANGBAND_DIR_SCORES, "scores.old");
	path_build(cur_name, sizeof(cur_name), ANGBAND_DIR_SCORES, "scores.raw");
	path_build(new_name, sizeof(new_name), ANGBAND_DIR_SCORES, "scores.new");
	path_build(lok_name, sizeof(lok_name), ANGBAND_DIR_SCORES, "scores.lok");


	/* Read in and add new score */
	n = highscore_count(scores, sz);


	/* Lock scores */
	if (file_exists(lok_name)) {
		msg("Lock file in place for scorefile; not writing.");
		return;
	}

	safe_setuid_grab();
	lok = file_open(lok_name, MODE_WRITE, FTYPE_RAW);
	file_lock(lok);
	safe_setuid_drop();

	if (!lok) {
		msg("Failed to create lock for scorefile; not writing.");
		return;
	}


	/* Open the new file for writing */
	safe_setuid_grab();
	scorefile = file_open(new_name, MODE_WRITE, FTYPE_RAW);
	safe_setuid_drop();

	if (!scorefile) {
		msg("Failed to open new scorefile for writing.");

		file_close(lok);
		file_delete(lok_name);
		return;
	}

	file_write(scorefile, (const char *)scores, sizeof(high_score)*n);
	file_close(scorefile);

	/* Now move things around */
	safe_setuid_grab();

	if (file_exists(old_name) && !file_delete(old_name))
		msg("Couldn't delete old scorefile");

	if (file_exists(cur_name) && !file_move(cur_name, old_name))
		msg("Couldn't move old scores.raw out of the way");

	if (!file_move(new_name, cur_name))
		msg("Couldn't rename new scorefile to scores.raw");

	/* Remove the lock */
	file_close(lok);
	file_delete(lok_name);

	safe_setuid_drop();
}



void build_score(high_score *entry, const char *died_from, time_t *death_time)
{
	memset(entry, 0, sizeof(high_score));

	/* Save the version */
	strnfmt(entry->what, sizeof(entry->what), "%s", buildid);

	/* Calculate and save the points */
	strnfmt(entry->pts, sizeof(entry->pts), "%9u", total_points());

	/* Save the current gold */
	strnfmt(entry->gold, sizeof(entry->gold), "%9u", player->au);

	/* Save the current turn */
	strnfmt(entry->turns, sizeof(entry->turns), "%9u", turn);

	/* Time of death */
	if (death_time)
		strftime(entry->day, sizeof(entry->day), "@%Y%m%d",
				 localtime(death_time));
	else
		my_strcpy(entry->day, "TODAY", sizeof(entry->day));

	/* Save the player name (15 chars) */
	strnfmt(entry->who, sizeof(entry->who), "%-.15s", op_ptr->full_name);

	/* Save the player info XXX XXX XXX */
	strnfmt(entry->uid, sizeof(entry->uid), "%7u", player_uid);
	strnfmt(entry->p_r, sizeof(entry->p_r), "%2d", player->race->ridx);
	strnfmt(entry->p_c, sizeof(entry->p_c), "%2d", player->class->cidx);

	/* Save the level and such */
	strnfmt(entry->cur_lev, sizeof(entry->cur_lev), "%3d", player->lev);
	strnfmt(entry->cur_dun, sizeof(entry->cur_dun), "%3d", player->depth);
	strnfmt(entry->max_lev, sizeof(entry->max_lev), "%3d", player->max_lev);
	strnfmt(entry->max_dun, sizeof(entry->max_dun), "%3d", player->max_depth);

	/* No cause of death */
	my_strcpy(entry->how, died_from, sizeof(entry->how));
}



/**
 * Enters a players name on a hi-score table, if "legal".
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
void enter_score(time_t *death_time)
{
	int j;

	/* Cheaters are not scored */
	for (j = 0; j < OPT_MAX; ++j) {
		if (option_type(j) != OP_SCORE)
			continue;
		if (!op_ptr->opt[j])
			continue;

		msg("Score not registered for cheaters.");
		event_signal(EVENT_MESSAGE_FLUSH);
		return;
	}

	/* Add a new entry, if allowed */
	if (player->noscore & (NOSCORE_WIZARD | NOSCORE_DEBUG)) {
		msg("Score not registered for wizards.");
		event_signal(EVENT_MESSAGE_FLUSH);
	} else if (!player->total_winner &&
			   streq(player->died_from, "Interrupting")) {
		msg("Score not registered due to interruption.");
		event_signal(EVENT_MESSAGE_FLUSH);
	} else if (!player->total_winner && streq(player->died_from, "Quitting")) {
		msg("Score not registered due to quitting.");
		event_signal(EVENT_MESSAGE_FLUSH);
	} else {
		high_score entry;
		high_score scores[MAX_HISCORES];

		build_score(&entry, player->died_from, death_time);

		highscore_read(scores, N_ELEMENTS(scores));
		highscore_add(&entry, scores, N_ELEMENTS(scores));
		highscore_write(scores, N_ELEMENTS(scores));
	}

	/* Success */
	return;
}


