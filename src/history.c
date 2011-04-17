/*
 * File: history.c
 * Purpose: Character auto-history creation, management, and display
 *
 * Copyright (c) 2007 J.D. White
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
#include "history.h"

/*
 * Number of slots available at birth in the player history list.  Defaults to
 * 10 and will expand automatically as new history entries are added, up the
 * the maximum defined value.
 */
#define HISTORY_BIRTH_SIZE  10
#define HISTORY_MAX 5000



/* The historical list for the character */
history_info *history_list;

/* Index of first writable entry */
static size_t history_ctr;

/* Current size of history list */
static size_t history_size;


#define LIMITLOW(a, b) if (a < b) a = b;
#define LIMITHI(a, b) if (a > b) a = b;



/*
 * Initialise an empty history list.
 */
static void history_init(size_t entries)
{
	history_ctr = 0;
	history_size = entries;
	history_list = C_ZNEW(history_size, history_info);
}


/*
 * Clear any existing history.
 */
void history_clear(void)
{
	if (!history_list) return;

	FREE(history_list);
	history_ctr = 0;
	history_size = 0;
}


/*
 * Set the number of history items.
 */
static bool history_set_num(size_t num)
{
	history_info *new_list;

	if (num > HISTORY_MAX)
		num = HISTORY_MAX;

	if (num < history_size)  return FALSE;
	if (num == history_size) return FALSE;

	/* Allocate new memory, copy across */
	/* XXX Should use mem_realloc() */
	new_list = C_ZNEW(num, history_info);
	C_COPY(new_list, history_list, history_ctr, history_info);
	FREE(history_list);

	history_list = new_list;
	history_size = num;

	return TRUE;
}


/*
 * Return the number of history entries.
 */
size_t history_get_num(void)
{
	return history_ctr;
}


/*
 * Mark artifact number `id` as known.
 */
static bool history_know_artifact(struct artifact *artifact)
{
	size_t i = history_ctr;
	assert(artifact);

	while (i--) {
		if (history_list[i].a_idx == artifact->aidx) {
			history_list[i].type = HISTORY_ARTIFACT_KNOWN;
			return TRUE;
		}
	}

	return FALSE;
}


/*
 * Mark artifact number `id` as lost forever, either due to leaving it on a
 * level, or due to a store purging its inventory after the player sold it.
 */
bool history_lose_artifact(struct artifact *artifact)
{
	size_t i = history_ctr;
	assert(artifact);

	while (i--) {
		if (history_list[i].a_idx == artifact->aidx) {
			history_list[i].type |= HISTORY_ARTIFACT_LOST;
			return TRUE;
		}
	}

	/* If we lost an artifact that didn't previously have a history, then we missed it */
	history_add_artifact(artifact, FALSE, FALSE);

	return FALSE;
}


/*
 * Add an entry with text `event` to the history list, with type `type`
 * ("HISTORY_xxx" in defines.h), and artifact number `id` (0 for everything
 * else).
 *
 * Return TRUE on success.
 */
bool history_add_full(u16b type, struct artifact *artifact, s16b dlev,
		s16b clev, s32b turn, const char *text)
{
	/* Allocate the history list if needed */
	if (!history_list)
		history_init(HISTORY_BIRTH_SIZE);

	/* Expand the history list if appropriate */
	else if ((history_ctr == history_size) && !history_set_num(history_size + 10))
		return FALSE;

	/* History list exists and is not full.  Add an entry at the current counter location. */
	history_list[history_ctr].type = type;
	history_list[history_ctr].dlev = dlev;
	history_list[history_ctr].clev = clev;
	history_list[history_ctr].a_idx = artifact ? artifact->aidx : 0;
	history_list[history_ctr].turn = turn;
	my_strcpy(history_list[history_ctr].event,
	          text, sizeof(history_list[history_ctr].event));

	history_ctr++;

	return TRUE;
}


/*
 * Add an entry with text `event` to the history list, with type `type`
 * ("HISTORY_xxx" in defines.h), and artifact number `id` (0 for everything
 * else).
 *
 * Returne TRUE on success.
 */
bool history_add(const char *event, u16b type, struct artifact *artifact)
{
	return history_add_full(type, artifact, p_ptr->depth, p_ptr->lev, turn, event);
}


/*
 * Returns TRUE if the artifact denoted by a_idx is KNOWN in the history log.
 */
bool history_is_artifact_known(struct artifact *artifact)
{
	size_t i = history_ctr;
	assert(artifact);

	while (i--) {
		if (history_list[i].type & HISTORY_ARTIFACT_KNOWN &&
				history_list[i].a_idx == artifact->aidx)
			return TRUE;
	}

	return FALSE;
}


/*
 * Returns TRUE if the artifact denoted by a_idx is an active entry in
 * the history log (i.e. is not marked HISTORY_ARTIFACT_LOST).  This permits
 * proper handling of the case where the player loses an artifact but (in
 * preserve mode) finds it again later.
 */
static bool history_is_artifact_logged(struct artifact *artifact)
{
	size_t i = history_ctr;
	assert(artifact);

	while (i--) {
		/* Don't count ARTIFACT_LOST entries; then we can handle
		 * re-finding previously lost artifacts in preserve mode  */
		if (history_list[i].type & HISTORY_ARTIFACT_LOST)
			continue;

		if (history_list[i].a_idx == artifact->aidx)
			return TRUE;
	}

	return FALSE;
}


/*
 * Adding artifacts to the history list is trickier than other operations.
 * This is a wrapper function that gets some of the logic out of places
 * where it really doesn't belong.  Call this to add an artifact to the history
 * list or make the history entry visible--history_add_artifact will make that
 * determination depending on what object_is_known returns for the artifact.
 */
bool history_add_artifact(struct artifact *artifact, bool known, bool found)
{
	object_type object_type_body;
	object_type *o_ptr = &object_type_body;

	char o_name[80];
	char buf[80];
	u16b type;

	assert(artifact);

	/* Make fake artifact for description purposes */
	object_wipe(o_ptr);
	make_fake_artifact(o_ptr, artifact);
	object_desc(o_name, sizeof(o_name), o_ptr,
				ODESC_PREFIX | ODESC_BASE | ODESC_SPOIL);
	strnfmt(buf, sizeof(buf), (found)?"Found %s":"Missed %s", o_name);

	/* Known objects gets different treatment */
	if (known) {
		/* Try revealing any existing artifact, otherwise log it */
		if (history_is_artifact_logged(artifact))
			history_know_artifact(artifact);
		else
			history_add(buf, HISTORY_ARTIFACT_KNOWN, artifact);
	} else {
		if (!history_is_artifact_logged(artifact)) {
			type = HISTORY_ARTIFACT_UNKNOWN |
					(found ? 0 : HISTORY_ARTIFACT_LOST);
			history_add(buf, type, artifact);
		} else {
			return FALSE;
		}
	}

	return TRUE;
}


/*
 * Convert all ARTIFACT_UNKNOWN history items to HISTORY_ARTIFACT_KNOWN.
 * Use only after player retirement/death for the final character dump.
 */
void history_unmask_unknown(void)
{
	size_t i = history_ctr;

	while (i--)
	{
		if (history_list[i].type & HISTORY_ARTIFACT_UNKNOWN)
		{
			history_list[i].type &= ~(HISTORY_ARTIFACT_UNKNOWN);
			history_list[i].type |= HISTORY_ARTIFACT_KNOWN;
		}
	}
}


/*
 * Used to determine whether the history entry is visible in the listing or not.
 * Returns TRUE if the item is masked -- that is, if it is invisible
 *
 * All artifacts are now sensed on pickup, so nothing is now invisible. The
 * KNOWN / UNKNOWN distinction is if we had fully identified it or not
 */
static bool history_masked(size_t i)
{
	return FALSE;
}

/*
 * Finds the index of the last printable (non-masked) item in the history list.
 */
static size_t last_printable_item(void)
{
	size_t i = history_ctr;

	while (i--)
	{
		if (!history_masked(i))
			break;
	}

	return i;
}

static void print_history_header(void)
{
	char buf[80];

	/* Print the header (character name and title) */
	strnfmt(buf, sizeof(buf), "%s the %s %s",
	        op_ptr->full_name,
	        p_ptr->race->name,
	        p_ptr->class->name);

	c_put_str(TERM_WHITE, buf, 0, 0);
	c_put_str(TERM_WHITE, "============================================================", 1, 0);
	c_put_str(TERM_WHITE, "                   CHAR.  ", 2, 0);
	c_put_str(TERM_WHITE, "|   TURN  | DEPTH |LEVEL| EVENT", 3, 0);
	c_put_str(TERM_WHITE, "============================================================", 4, 0);
}


/* Handles all of the display functionality for the history list. */
void history_display(void)
{
	int row, wid, hgt, page_size;
	char buf[90];
	static size_t first_item = 0;
	size_t max_item = last_printable_item();
	size_t i;

	Term_get_size(&wid, &hgt);

	/* Six lines provide space for the header and footer */
	page_size = hgt - 6;

	screen_save();

	while (1)
	{
		struct keypress ch;

		Term_clear();

		/* Print everything to screen */
		print_history_header();

		row = 0;
		for (i = first_item; row <= page_size && i < history_ctr; i++)
		{
			/* Skip messages about artifacts not yet IDed. */
			if (history_masked(i))
				continue;

			strnfmt(buf, sizeof(buf), "%10d%7d\'%5d   %s",
				history_list[i].turn,
				history_list[i].dlev * 50,
				history_list[i].clev,
				history_list[i].event);

			if (history_list[i].type & HISTORY_ARTIFACT_LOST)
				my_strcat(buf, " (LOST)", sizeof(buf));

			/* Size of header = 5 lines */
			prt(buf, row + 5, 0);
			row++;
		}
		prt("[Arrow keys scroll, p for previous page, n for next page, ESC to exit.]", hgt - 1, 0);

		ch = inkey();

		/* XXXmacro we should have a generic "key -> scroll" function */
		if (ch.code == 'n')
		{
			size_t scroll_to = first_item + page_size;

			while (history_masked(scroll_to) && scroll_to < history_ctr - 1)
				scroll_to++;

			first_item = (scroll_to < max_item ? scroll_to : max_item);
		}
		else if (ch.code == 'p')
		{
			int scroll_to = first_item - page_size;

			while (history_masked(scroll_to) && scroll_to > 0)
				scroll_to--;

			first_item = (scroll_to >= 0 ? scroll_to : 0);
		}
		else if (ch.code == ARROW_DOWN)
		{
			size_t scroll_to = first_item + 1;

			while (history_masked(scroll_to) && scroll_to < history_ctr - 1)
				scroll_to++;

			first_item = (scroll_to < max_item ? scroll_to : max_item);
		}
		else if (ch.code == ARROW_UP)
		{
			int scroll_to = first_item - 1;

			while (history_masked(scroll_to) && scroll_to > 0)
				scroll_to--;

			first_item = (scroll_to >= 0 ? scroll_to : 0);
		}
		else if (ch.code == ESCAPE)
			break;
	}

	screen_load();

	return;
}


/* Dump character history to a file, which we assume is already open. */
void dump_history(ang_file *file)
{
	size_t i;
	char buf[90];

	/* We use either ascii or system-specific encoding */
 	int encoding = OPT(xchars_to_file) ? SYSTEM_SPECIFIC : ASCII;

        file_putf(file, "============================================================\n");
        file_putf(file, "                   CHAR.\n");
        file_putf(file, "|   TURN  | DEPTH |LEVEL| EVENT\n");
        file_putf(file, "============================================================\n");

	for (i = 0; i < (last_printable_item() + 1); i++)
	{
		/* Skip not-yet-IDd artifacts */
		if (history_masked(i)) continue;

                strnfmt(buf, sizeof(buf), "%10d%7d\'%5d   %s",
                                history_list[i].turn,
                                history_list[i].dlev * 50,
                                history_list[i].clev,
                                history_list[i].event);

                if (history_list[i].type & HISTORY_ARTIFACT_LOST)
                                my_strcat(buf, " (LOST)", sizeof(buf));

		x_file_putf(file, encoding, "%s", buf);
		file_put(file, "\n");
	}

	return;
}
