/**
 * \file player-history.c
 * \brief Character auto-history creation, management, and display
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
#include "cave.h"
#include "game-world.h"
#include "obj-desc.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-history.h"

/**
 * Memory allocation constants.
 */
#define HISTORY_LEN_INIT		20
#define HISTORY_LEN_INCR		20

/**
 * Initialise an empty history list.
 */
static void history_init(struct player_history *h)
{
	h->next = 0;
	h->length = HISTORY_LEN_INIT;
	h->entries = mem_zalloc(h->length * sizeof(*h->entries));
}

/**
 * Increase the history array size.
 */
static void history_realloc(struct player_history *h)
{
	h->length += HISTORY_LEN_INCR;
	h->entries = mem_realloc(h->entries,
			h->length * sizeof *h->entries);
}

/**
 * Clear any existing history.
 */
void history_clear(struct player *p)
{
	struct player_history *h = &p->hist;

	if (h->entries) {
		mem_free(h->entries);
		h->entries = NULL;
	}

	h->next = 0;
	h->length = 0;
}

/**
 * Add an entry with text `text` to the history list, with type `type`
 * ("HIST_xxx" in player-history.h), and artifact number `id` (0 for
 * everything else).
 *
 * Return true on success.
 */
bool history_add_full(struct player *p,
		bitflag *type,
		int aidx,
		int dlev,
		int clev,
		int turnno,
		const char *text)
{
	struct player_history *h = &p->hist;

	/* Allocate or expand the history list if needed */
	if (!h->entries)
		history_init(h);
	else if (h->next == h->length)
		history_realloc(h);

	/* Add entry */
	hist_copy(h->entries[h->next].type, type);
	h->entries[h->next].dlev = dlev;
	h->entries[h->next].clev = clev;
	h->entries[h->next].a_idx = aidx;
	h->entries[h->next].turn = turnno;
	my_strcpy(h->entries[h->next].event,
			text,
			sizeof(h->entries[h->next].event));

	h->next++;

	return true;
}

/**
 * Add an entry to the history ledger with specified bitflags.
 */
static bool history_add_with_flags(struct player *p,
		const char *text,
		bitflag flags[HIST_SIZE],
		const struct artifact *artifact)
{
	return history_add_full(p,
		flags,
		artifact ? artifact->aidx : 0,
		p->depth,
		p->lev,
		p->total_energy / 100,
		text);
}

/**
 * Adds an entry to the history ledger.
 */
bool history_add(struct player *p, const char *text, int type)
{
	bitflag flags[HIST_SIZE];
	hist_wipe(flags);
	hist_on(flags, type);

	return history_add_with_flags(p, text, flags, NULL);
}

/**
 * Returns true if the artifact is KNOWN in the history log.
 */
bool history_is_artifact_known(struct player *p, const struct artifact *artifact)
{
	struct player_history *h = &p->hist;

	size_t i = h->next;
	assert(artifact);

	while (i--) {
		if (hist_has(h->entries[i].type, HIST_ARTIFACT_KNOWN) &&
				h->entries[i].a_idx == artifact->aidx)
			return true;
	}

	return false;
}

/**
 * Mark artifact as known.
 */
static bool history_mark_artifact_known(struct player_history *h,
		const struct artifact *artifact)
{
	assert(artifact);

	size_t i = h->next;
	while (i--) {
		if (h->entries[i].a_idx == artifact->aidx) {
			hist_off(h->entries[i].type, HIST_ARTIFACT_UNKNOWN);
			hist_on(h->entries[i].type, HIST_ARTIFACT_KNOWN);
			return true;
		}
	}

	return false;
}

/**
 * Mark artifact as lost.
 */
static bool history_mark_artifact_lost(struct player_history *h,
		const struct artifact *artifact)
{
	assert(artifact);

	size_t i = h->next;
	while (i--) {
		if (h->entries[i].a_idx == artifact->aidx) {
			hist_on(h->entries[i].type, HIST_ARTIFACT_LOST);
			return true;
		}
	}

	return false;
}

/**
 * Utility function for history_add_artifact(): get artifact name
 */
static void get_artifact_name(char *buf, size_t len, const struct artifact *artifact)
{
	struct object body = OBJECT_NULL;
	struct object known_body = OBJECT_NULL;

	struct object *fake = &body;
	struct object *known_obj = &known_body;

	/* Make fake artifact for description purposes */
	make_fake_artifact(fake, artifact);

	fake->known = known_obj;
	object_copy(known_obj, fake);
	object_desc(buf, len, fake, ODESC_PREFIX | ODESC_BASE | ODESC_SPOIL);

	object_wipe(known_obj);
	object_wipe(fake);
}

/**
 * Add an artifact to the history log.
 *
 * Call this to add an artifact to the history list or make the history
 * entry visible.
 */
void history_find_artifact(struct player *p, const struct artifact *artifact)
{
	assert(artifact != NULL);

	/* Try revealing any existing artifact, otherwise log it */
	if (!history_mark_artifact_known(&p->hist, artifact)) {
		char o_name[80];
		char text[80];

		get_artifact_name(o_name, sizeof(o_name), artifact);
		strnfmt(text, sizeof(text), "Found %s", o_name);

		bitflag flags[HIST_SIZE];
		hist_wipe(flags);
		hist_on(flags, HIST_ARTIFACT_KNOWN);

		history_add_with_flags(p, text, flags, artifact);
	}
}

/**
 * Mark artifact number `id` as lost forever.
 */
void history_lose_artifact(struct player *p, const struct artifact *artifact)
{
	assert(artifact != NULL);

	/* Try to mark it as lost if it's already in history */
	if (!history_mark_artifact_lost(&p->hist, artifact)) {
		/* Otherwise add a new entry */
		char o_name[80];
		char text[80];

		get_artifact_name(o_name, sizeof(o_name), artifact);
		strnfmt(text, sizeof(text), "Missed %s", o_name);

		bitflag flags[HIST_SIZE];
		hist_wipe(flags);
		hist_on(flags, HIST_ARTIFACT_UNKNOWN);
		hist_on(flags, HIST_ARTIFACT_LOST);

		history_add_with_flags(p, text, flags, artifact);
	}
}

/**
 * Convert all ARTIFACT_UNKNOWN history items to HIST_ARTIFACT_KNOWN.
 * Use only after player retirement/death for the final character dump.
 */
void history_unmask_unknown(struct player *p)
{
	struct player_history *h = &p->hist;

	size_t i = h->next;
	while (i--) {
		if (hist_has(h->entries[i].type, HIST_ARTIFACT_UNKNOWN)) {
			hist_off(h->entries[i].type, HIST_ARTIFACT_UNKNOWN);
			hist_on(h->entries[i].type, HIST_ARTIFACT_KNOWN);
		}
	}
}

/**
 * Present a copy of the history fot UI use
 */
size_t history_get_list(struct player *p, struct history_info **list)
{
	struct player_history *h = &p->hist;

	*list = h->entries;
	return h->next;
}
