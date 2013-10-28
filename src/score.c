/*
 * File: score.c
 * Purpose: Highscore handling for Angband
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


/*
 * Maximum number of high scores in the high score file
 */
#define MAX_HISCORES    100


/*
 * Semi-Portable High Score List Entry (128 bytes)
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */
typedef struct
{
	char what[8];		/* Version info (string) */

	char pts[10];		/* Total Score (number) */

	char gold[10];		/* Total Gold (number) */

	char turns[10];		/* Turns Taken (number) */

	char day[10];		/* Time stamp (string) */

	char who[16];		/* Player Name (string) */

	char uid[8];		/* Player UID (number) */

	char sex[2];		/* Player Sex (string) */
	char p_r[3];		/* Player Race (number) */
	char p_c[3];		/* Player Class (number) */

	char cur_lev[4];		/* Current Player Level (number) */
	char cur_dun[4];		/* Current Dungeon Level (number) */
	char max_lev[4];		/* Max Player Level (number) */
	char max_dun[4];		/* Max Dungeon Level (number) */

	char how[32];		/* Method of death (string) */
} high_score;



/*
 * Hack -- Calculates the total number of points earned
 */
static long total_points(void)
{
	return (p_ptr->max_exp + (100 * p_ptr->max_depth));
}


/*
 * Read in a highscore file.
 */
static size_t highscore_read(high_score scores[], size_t sz)
{
	char fname[1024];
	ang_file *scorefile;
	size_t i;

	/* Wipe current scores */
	C_WIPE(scores, sz, high_score);

	path_build(fname, sizeof(fname), ANGBAND_DIR_APEX, "scores.raw");
	scorefile = file_open(fname, MODE_READ, -1);

	if (!scorefile) return TRUE;

	for (i = 0; i < sz &&
			file_read(scorefile, (char *)&scores[i], sizeof(high_score)) > 0;
			i++)
		;

	file_close(scorefile);

	return i;
}


/*
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
static size_t highscore_where(const high_score *entry, const high_score scores[], size_t sz)
{
	size_t i;

	/* Read until we get to a higher score */
	for (i = 0; i < sz; i++)
	{
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

static size_t highscore_add(const high_score *entry, high_score scores[], size_t sz)
{
	size_t slot = highscore_where(entry, scores, sz);

	memmove(&scores[slot+1], &scores[slot], sizeof(high_score) * (sz - 1 - slot));
	memcpy(&scores[slot], entry, sizeof(high_score));

	return slot;
}

static size_t highscore_count(const high_score scores[], size_t sz)
{
	size_t i;
	for (i = 0; i < sz; i++)
	{
		if (scores[i].what[0] == '\0') break;
	}

	return i;
}


/*
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

	path_build(old_name, sizeof(old_name), ANGBAND_DIR_APEX, "scores.old");
	path_build(cur_name, sizeof(cur_name), ANGBAND_DIR_APEX, "scores.raw");
	path_build(new_name, sizeof(new_name), ANGBAND_DIR_APEX, "scores.new");
	path_build(lok_name, sizeof(lok_name), ANGBAND_DIR_APEX, "scores.lok");


	/* Read in and add new score */
	n = highscore_count(scores, sz);


	/*** Lock scores ***/

	if (file_exists(lok_name))
	{
		msg("Lock file in place for scorefile; not writing.");
		return;
	}

	safe_setuid_grab();
	lok = file_open(lok_name, MODE_WRITE, FTYPE_RAW);
	file_lock(lok);
	safe_setuid_drop();

	if (!lok)
	{
		msg("Failed to create lock for scorefile; not writing.");
		return;
	}


	/*** Open the new file for writing ***/

	safe_setuid_grab();
	scorefile = file_open(new_name, MODE_WRITE, FTYPE_RAW);
	safe_setuid_drop();

	if (!scorefile)
	{
		msg("Failed to open new scorefile for writing.");

		file_close(lok);
		file_delete(lok_name);
		return;
	}

	file_write(scorefile, (const char *)scores, sizeof(high_score)*n);
	file_close(scorefile);

	/*** Now move things around ***/

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



/*
 * Display the scores in a given range.
 */
static void display_scores_aux(const high_score scores[], int from, int to, int highlight)
{
	struct keypress ch;

	int j, k, n, place;
	int count;


	/* Assume we will show the first 10 */
	if (from < 0) from = 0;
	if (to < 0) to = 10;
	if (to > MAX_HISCORES) to = MAX_HISCORES;


	/* Hack -- Count the high scores */
	for (count = 0; count < MAX_HISCORES; count++)
	{
		if (!scores[count].what[0]) break;
	}

	/* Forget about the last entries */
	if (count > to) count = to;


	/* Show 5 per page, until "done" */
	for (k = from, j = from, place = k+1; k < count; k += 5)
	{
		char out_val[160];
		char tmp_val[160];

		/* Clear screen */
		Term_clear();

		/* Title */
		if (k > 0)
			put_str(format("%s Hall of Fame (from position %d)", VERSION_NAME, place), 0, 21);
		else
			put_str(format("%s Hall of Fame", VERSION_NAME), 0, 30);


		/* Dump 5 entries */
		for (n = 0; j < count && n < 5; place++, j++, n++)
		{
			const high_score *score = &scores[j];

			byte attr;

			int clev, mlev, cdun, mdun;
			const char *user, *gold, *when, *aged;
			struct player_class *c;
			struct player_race *r;


			/* Hack -- indicate death in yellow */
			attr = (j == highlight) ? TERM_L_GREEN : TERM_WHITE;

			c = player_id2class(atoi(score->p_c));
			r = player_id2race(atoi(score->p_r));

			/* Extract the level info */
			clev = atoi(score->cur_lev);
			mlev = atoi(score->max_lev);
			cdun = atoi(score->cur_dun);
			mdun = atoi(score->max_dun);

			/* Hack -- extract the gold and such */
			for (user = score->uid; isspace((unsigned char)*user); user++) /* loop */;
			for (when = score->day; isspace((unsigned char)*when); when++) /* loop */;
			for (gold = score->gold; isspace((unsigned char)*gold); gold++) /* loop */;
			for (aged = score->turns; isspace((unsigned char)*aged); aged++) /* loop */;

			/* Dump some info */
			strnfmt(out_val, sizeof(out_val),
			        "%3d.%9s  %s the %s %s, Level %d",
			        place, score->pts, score->who,
			        r ? r->name : "<none>", c ? c->name : "<none>",
			        clev);

			/* Append a "maximum level" */
			if (mlev > clev) my_strcat(out_val, format(" (Max %d)", mlev), sizeof(out_val));

			/* Dump the first line */
			c_put_str(attr, out_val, n*4 + 2, 0);


			/* Died where? */
			if (!cdun)
				strnfmt(out_val, sizeof(out_val), "Killed by %s in the town", score->how);
			else
				strnfmt(out_val, sizeof(out_val), "Killed by %s on dungeon level %d", score->how, cdun);

			/* Append a "maximum level" */
			if (mdun > cdun)
				my_strcat(out_val, format(" (Max %d)", mdun), sizeof(out_val));

			/* Dump the info */
			c_put_str(attr, out_val, n*4 + 3, 15);


			/* Clean up standard encoded form of "when" */
			if ((*when == '@') && strlen(when) == 9)
			{
				strnfmt(tmp_val, sizeof(tmp_val), "%.4s-%.2s-%.2s", when + 1, when + 5, when + 7);
				when = tmp_val;
			}

			/* And still another line of info */
			strnfmt(out_val, sizeof(out_val), "(User %s, Date %s, Gold %s, Turn %s).", user, when, gold, aged);
			c_put_str(attr, out_val, n*4 + 4, 15);
		}


		/* Wait for response */
		prt("[Press ESC to exit, any other key to continue.]", 23, 17);
		ch = inkey();
		prt("", 23, 0);

		/* Hack -- notice Escape */
		if (ch.code == ESCAPE) break;
	}

	return;
}

static void build_score(high_score *entry, const char *died_from, time_t *death_time)
{
	WIPE(entry, high_score);

	/* Save the version */
	strnfmt(entry->what, sizeof(entry->what), "%s", buildid);

	/* Calculate and save the points */
	strnfmt(entry->pts, sizeof(entry->pts), "%9lu", (long)total_points());

	/* Save the current gold */
	strnfmt(entry->gold, sizeof(entry->gold), "%9lu", (long)p_ptr->au);

	/* Save the current turn */
	strnfmt(entry->turns, sizeof(entry->turns), "%9lu", (long)turn);

	/* Time of death */
	if (death_time)
		strftime(entry->day, sizeof(entry->day), "@%Y%m%d", localtime(death_time));
	else
		my_strcpy(entry->day, "TODAY", sizeof(entry->day));

	/* Save the player name (15 chars) */
	strnfmt(entry->who, sizeof(entry->who), "%-.15s", op_ptr->full_name);

	/* Save the player info XXX XXX XXX */
	strnfmt(entry->uid, sizeof(entry->uid), "%7u", player_uid);
	strnfmt(entry->sex, sizeof(entry->sex), "%c", (p_ptr->psex ? 'm' : 'f'));
	strnfmt(entry->p_r, sizeof(entry->p_r), "%2d", p_ptr->race->ridx);
	strnfmt(entry->p_c, sizeof(entry->p_c), "%2d", p_ptr->class->cidx);

	/* Save the level and such */
	strnfmt(entry->cur_lev, sizeof(entry->cur_lev), "%3d", p_ptr->lev);
	strnfmt(entry->cur_dun, sizeof(entry->cur_dun), "%3d", p_ptr->depth);
	strnfmt(entry->max_lev, sizeof(entry->max_lev), "%3d", p_ptr->max_lev);
	strnfmt(entry->max_dun, sizeof(entry->max_dun), "%3d", p_ptr->max_depth);

	/* No cause of death */
	my_strcpy(entry->how, died_from, sizeof(entry->how));
}



/*
 * Enters a players name on a hi-score table, if "legal".
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
void enter_score(time_t *death_time)
{
	int j;

	/* Cheaters are not scored */
	for (j = OPT_SCORE; j < OPT_SCORE + N_OPTS_CHEAT; ++j)
	{
		if (!op_ptr->opt[j]) continue;

		msg("Score not registered for cheaters.");
		message_flush();
		return;
	}

	/* Wizard-mode pre-empts scoring */
	if (p_ptr->noscore & (NOSCORE_WIZARD | NOSCORE_DEBUG))
	{
		msg("Score not registered for wizards.");
		message_flush();
	}

#ifndef SCORE_BORGS

	/* Borg-mode pre-empts scoring */
	else if (p_ptr->noscore & NOSCORE_BORG)
	{
		msg("Score not registered for borgs.");
		message_flush();
	}

#endif /* SCORE_BORGS */

	/* Hack -- Interupted */
	else if (!p_ptr->total_winner && streq(p_ptr->died_from, "Interrupting"))
	{
		msg("Score not registered due to interruption.");
		message_flush();
	}

	/* Hack -- Quitter */
	else if (!p_ptr->total_winner && streq(p_ptr->died_from, "Quitting"))
	{
		msg("Score not registered due to quitting.");
		message_flush();
	}

	/* Add a new entry to the score list, see where it went */
	else
	{
		high_score entry;
		high_score scores[MAX_HISCORES];

		build_score(&entry, p_ptr->died_from, death_time);

		highscore_read(scores, N_ELEMENTS(scores));
		highscore_add(&entry, scores, N_ELEMENTS(scores));
		highscore_write(scores, N_ELEMENTS(scores));
	}

	/* Success */
	return;
}


/*
 * Predict the players location, and display it.
 */
void predict_score(void)
{
	int j;
	high_score the_score;

	high_score scores[MAX_HISCORES];


	/* Read scores, place current score */
	highscore_read(scores, N_ELEMENTS(scores));
	build_score(&the_score, "nobody (yet!)", NULL);

	if (p_ptr->is_dead)
		j = highscore_where(&the_score, scores, N_ELEMENTS(scores));
	else
		j = highscore_add(&the_score, scores, N_ELEMENTS(scores));

	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(scores, 0, 15, j);
	}

	/* Display some "useful" scores */
	else
	{
		display_scores_aux(scores, 0, 5, -1);
		display_scores_aux(scores, j - 2, j + 7, j);
	}
}


/*
 * Show scores.
 */
void show_scores(void)
{
	screen_save();

	/* Display the scores */
	if (character_generated)
	{
		predict_score();
	}
	else
	{
		high_score scores[MAX_HISCORES];
		highscore_read(scores, N_ELEMENTS(scores));
		display_scores_aux(scores, 0, MAX_HISCORES, -1);
	}

	screen_load();

	/* Hack - Flush it */
	Term_fresh();
}

