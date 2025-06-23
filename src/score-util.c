/**
 * \file score-util.c
 * \brief Define score functions shared by the game's core and the separate
 * elevated privilege executable.
 *
 * Copyright (c) 2025 Ben Harrison, James E. Wilson, Robert A. Koeneke,
 * Eric Branlund
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

/*
 * To simplify auditing and linking of the elevated privilege executable,
 * functions here should not call into anything else besides what is in this
 * file or in the C standard library.
 */

#include "score.h"
#include <assert.h>	/* assert */
#include <stdlib.h>	/* qsort, strtol */
#include <string.h>	/* memchr, memset, strcmp */


/**
 * Compare two high score records.
 *
 * \param a is a pointer to a high score record cast to a const void*.
 * \param b is a pointer to a high score record cast to a const void*.
 * \return -1 if *a should appear before *b.  Return +1 if *a should appear
 * after *b.  Otherwise, return zero.
 *
 * Assumes both records are valid and elements in the same array.
 */
static int highscore_cmp(const void *a, const void *b)
{
	const struct high_score *sa = (const struct high_score*)a;
	const struct high_score *sb = (const struct high_score*)b;
	int result;

	/*
	 * Empty records appear after non-empty ones.  Do not care about
	 * the order when both are empty.
	 */
	if (!sa->what[0]) {
		result = (sb->what[0]) ? 1 : 0;
	} else if (!sb->what[0]) {
		result = (sa->what[0]) ? -1 : 0;
	} else {
		/* A winning record appears before any that did not win. */
		bool awinner = strcmp(sa->how, WINNING_HOW) == 0;
		bool bwinner = strcmp(sb->how, WINNING_HOW) == 0;

		if (awinner != bwinner) {
			result = (awinner) ? -1 : 1;
		} else {
			/*
			 * A record with more points appears before any
			 * with the same winning status and less points.
			 */
			long apts = strtol(sa->pts, NULL, 0);
			long bpts = strtol(sb->pts, NULL, 0);

			if (apts != bpts) {
				result = (apts > bpts) ? -1 : 1;
			} else {
				/*
				 * With same winning status and points, keep
				 * the same order as the records currently
				 * have.
				 */
				result = (sa < sb) ? -1 : ((sa > sb) ? 1 : 0);
			}
		}
	}
	return result;
}


/**
 * Check that a high score record can be used without triggering undefined
 * behavior or failures in integer parsing.
 *
 * \param s points to the score record to check.
 * \return true if the record is valid.  Otherwise, return false.
 */
bool highscore_valid(const struct high_score *s)
{
	if (s->what[0]) {
		/* It is a non-empty record. */
		char *p_end;

		/* All fields must be null-terminated. */
		if (!memchr(s->what, '\0', sizeof(s->what))
			|| !memchr(s->pts, '\0', sizeof(s->pts))
			|| !memchr(s->gold, '\0', sizeof(s->gold))
			|| !memchr(s->turns, '\0', sizeof(s->turns))
			|| !memchr(s->day, '\0', sizeof(s->day))
			|| !memchr(s->who, '\0', sizeof(s->who))
			|| !memchr(s->uid, '\0', sizeof(s->uid))
			|| !memchr(s->p_r, '\0', sizeof(s->p_r))
			|| !memchr(s->p_c, '\0', sizeof(s->p_c))
			|| !memchr(s->cur_lev, '\0', sizeof(s->cur_lev))
			|| !memchr(s->cur_dun, '\0', sizeof(s->cur_dun))
			|| !memchr(s->max_lev, '\0', sizeof(s->max_lev))
			|| !memchr(s->max_dun, '\0', sizeof(s->max_dun))
			|| !memchr(s->how, '\0', sizeof(s->how))) {
			return false;
		}
		/*
		 * It must have number values that are parseable.  Except for
		 * pts, gold, turns, and uid, the fields are short enough that
		 * they will not reach the most restrictive bounds for an int
		 * with c99.  For pts, gold, turns, and uid, the fields are
		 * short enough that they will noot reach most restrictive
		 * bounds for a long with c99.
		 */
		(void)strtol(s->pts, &p_end, 0);
		if (*p_end || !s->pts[0]) {
			return false;
		}
		(void)strtol(s->gold, &p_end, 0);
		if (*p_end || !s->gold[0]) {
			return false;
		}
		(void)strtol(s->turns, &p_end, 0);
		if (*p_end || !s->turns[0]) {
			return false;
		}
		(void)strtol(s->uid, &p_end, 0);
		if (*p_end || !s->uid[0]) {
			return false;
		}
		(void)strtol(s->p_r, &p_end, 0);
		if (*p_end || !s->p_r[0]) {
			return false;
		}
		(void)strtol(s->p_c, &p_end, 0);
		if (*p_end || !s->p_c[0]) {
			return false;
		}
		(void)strtol(s->cur_lev, &p_end, 0);
		if (*p_end || !s->cur_lev[0]) {
			return false;
		}
		(void)strtol(s->cur_dun, &p_end, 0);
		if (*p_end || !s->cur_dun[0]) {
			return false;
		}
		(void)strtol(s->max_lev, &p_end, 0);
		if (*p_end || !s->max_lev[0]) {
			return false;
		}
		(void)strtol(s->max_dun, &p_end, 0);
		if (*p_end || !s->max_dun[0]) {
			return false;
		}
	} else {
		/*
		 * It is an empty record.  All the fields must be empty
		 * strings.
		 */
		if (s->pts[0]
				|| s->gold[0]
				|| s->turns[0]
				|| s->day[0]
				|| s->who[0]
				|| s->uid[0]
				|| s->p_r[0]
				|| s->p_c[0]
				|| s->cur_lev[0]
				|| s->cur_dun[0]
				|| s->max_lev[0]
				|| s->max_dun[0]
				|| s->how[0]) {
			return false;
		}
	}
	return true;
}


/**
 * Force an array of high scores to all be valid and in the proper order.
 *
 * \param scores is the array of high scores.
 * \param sz is the number of high scores in scores.  It may be zero.
 * \return true if the contents of scores were modified due to irregularities.
 * Otherwise, return false.
 */
bool highscore_regularize(struct high_score scores[], size_t sz)
{
	bool irregular = false, out_of_order = false;
	const struct high_score *last_not_empty = NULL;
	/*
	 * Inclusive bounds for the indices that were copied out of and not
	 * subsequently overwritten
	 */
	size_t first_copied = sz, last_copied = 0;
	size_t i_in, i_out;

	for (i_in = 0, i_out = 0; i_in < sz; ++i_in) {
		if (!highscore_valid(scores + i_in)) {
			/* Make an invalid score empty. */
			(void)memset(scores + i_in, 0, sizeof(scores[i_in]));
			irregular = true;
			continue;
		}
		/* Skip empty score records. */
		if (!scores[i_in].what[0]) {
			continue;
		}
		if (last_not_empty && highscore_cmp(last_not_empty,
				scores + i_in) > 0) {
			out_of_order = true;
		}
		if (i_out != i_in) {
			/*
			 * Copy the record to cover gaps from invalid or
			 * empty records.
			 */
			scores[i_out] = scores[i_in];
			if (first_copied > i_in) {
				first_copied = i_in;
			} else if (first_copied == i_out) {
				++first_copied;
			}
			last_copied = i_in;
			irregular = true;
		}
		last_not_empty = scores + i_in;
		++i_out;
	}
	if (out_of_order) {
		/*
		 * Can only be out of order if there are two or more records
		 * that are not empty.
		 */
		assert(i_out >= 2 && i_out <= sz);
		qsort(scores, i_out, sizeof(scores[0]), highscore_cmp);
		irregular = true;
	}
	if (first_copied <= last_copied) {
		/*
		 * Copied records before.  Now guarantee that the records
		 * copied but not overwritten are empty.
		 */
		assert(last_copied < sz);
		(void)memset(scores + first_copied, 0,
			(last_copied - first_copied + 1)
			* sizeof(scores[i_out]));
	}
	return irregular;
}


/**
 * Determine where a new score would be placed.
 *
 * \param entry points to the new score which is valid and not empty.
 * \param scores is an array of sz valid score records, sorted from best to
 * worst with any empty records at the end.
 * \param sz is the number of score records scores holds.  sz must be positive.
 * \return the index in scores, zero to sz - 1, where *entry belongs.  Zero
 * is best, and the last entry can always be overwritten, even if its score
 * is better than that of *entry.
 */
size_t highscore_where(const struct high_score *entry,
		const struct high_score scores[], size_t sz)
{
	/*
	 * Read until we get to a higher score.  The score comparison should
	 * be the same as highscore_cmp() does except that for ties, we give
	 * preference to the newer score, the one in *entry.
	 */
	bool entry_winner = strcmp(entry->how, WINNING_HOW) == 0;
	long entry_pts = strtol(entry->pts, NULL, 0);
	size_t i;

	for (i = 0; i < sz; ++i) {
		bool score_winner;
		long score_pts;

		/*
		 * *entry is not empty so it would replace the first empty
		 * record.
		 */
		if (!scores[i].what[0]) {
			return i;
		}

		/* A winning record appears before any that did not win. */
		score_winner = strcmp(scores[i].how, WINNING_HOW) == 0;
		if (entry_winner != score_winner) {
			if (entry_winner) {
				return i;
			}
			continue;
		}

		/*
		 * A record with more points appears before any with the
		 * the same winning status and less points.  If a tie, put
		 * *entry first.
		 */
		score_pts = strtol(scores[i].pts, NULL, 0);
		if (entry_pts >= score_pts) {
			return i;
		}
	}

	/* If it was not a fit elsewhere, replace the last entry. */
	return sz - 1;
}
