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
 * Number of slots available at birth in the player history list.
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
	h->length = h->length + HISTORY_LEN_INCR;
	h->entries = mem_realloc(h->entries,
			h->length * sizeof *h->entries);
}

/**
 * Clear any existing history.
 */
void history_clear(struct player *p)
{
	struct player_history *h = &player->hist;

	if (h->entries) {
		mem_free(h->entries);
	}

	h->entries = NULL;
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
		const struct artifact *artifact,
		s16b dlev,
		s16b clev,
		s32b turnno,
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
	h->entries[h->next].a_idx = artifact ? artifact->aidx : 0;
	h->entries[h->next].turn = turnno;
	my_strcpy(h->entries[h->next].event,
			text,
			sizeof(h->entries[h->next].event));

	h->next++;

	return true;
}

/**
 * Add an entry with text `text` to the history list, with type `type`
 * ("HIST_xxx" in player-history.h), and artifact number `id` (0 for
 * everything else).
 *
 * Return true on success.
 */
bool history_add(struct player *p,
		const char *text,
		int type,
		const struct artifact *artifact)
{
	bitflag flags[HIST_SIZE];
	hist_wipe(flags);
	hist_on(flags, type);

	return history_add_full(p,
		flags,
		artifact,
		p->depth,
		p->lev,
		p->total_energy / 100,
		text);
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
 * Returns true if the artifact denoted by a_idx is an active entry in
 * the history log (i.e. is not marked HIST_ARTIFACT_LOST).  This permits
 * proper handling of the case where the player loses an artifact but (in
 * preserve mode) finds it again later.
 */
static bool history_is_artifact_logged(struct player *p, const struct artifact *artifact)
{
	assert(artifact);

	struct player_history *h = &p->hist;

	size_t i = h->next;
	while (i--) {
		struct history_info *entry = &h->entries[i];

		/* Don't count ARTIFACT_LOST entries; then we can handle
		 * re-finding previously lost artifacts in preserve mode  */
		if (entry->a_idx == artifact->aidx &&
					!hist_has(entry->type, HIST_ARTIFACT_LOST)) {
			return true;
		}
	}

	return false;
}

/**
 * Mark artifact number `id` as known.
 */
static bool history_mark_artifact_known(struct player_history *h,
		const struct artifact *artifact)
{
	assert(artifact);

	size_t i = h->next;
	while (i--) {
		if (h->entries[i].a_idx == artifact->aidx) {
			hist_wipe(h->entries[i].type);
			hist_on(h->entries[i].type, HIST_ARTIFACT_KNOWN);
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

	object_wipe(known_obj, false);
	object_wipe(fake, true);
}

/**
 * Add an artifact to the history log.
 *
 * Call this to add an artifact to the history list or make the history
 * entry visible.
 */
bool history_add_artifact(struct player *p,
		const struct artifact *artifact,
		bool known,
		bool found)
{
	assert(artifact != NULL);

	char o_name[80];
	char buf[80];

	get_artifact_name(o_name, sizeof(o_name), artifact);
	strnfmt(buf, sizeof(buf), found ? "Found %s" : "Missed %s", o_name);

	/* Known objects gets different treatment */
	if (known) {
		/* Try revealing any existing artifact, otherwise log it */
		if (history_is_artifact_logged(p, artifact)) {
			history_mark_artifact_known(&p->hist, artifact);
		} else {
			history_add(p, buf, HIST_ARTIFACT_KNOWN, artifact);
		}
	} else {
		if (!history_is_artifact_logged(p, artifact)) {
			bitflag type[HIST_SIZE];
			hist_wipe(type);
			hist_on(type, HIST_ARTIFACT_UNKNOWN);
			if (!found) {
				hist_on(type, HIST_ARTIFACT_LOST);
			}

			history_add_full(p, type, artifact, player->depth, player->lev,
							 player->total_energy / 100, buf);
		} else {
			return false;
		}
	}

	return true;
}

/**
 * Mark artifact number `id` as lost forever.
 */
bool history_lose_artifact(struct player *p, const struct artifact *artifact)
{
	assert(artifact);

	struct player_history *h = &p->hist;

	size_t i = h->next;
	while (i--) {
		if (h->entries[i].a_idx == artifact->aidx) {
			hist_on(h->entries[i].type, HIST_ARTIFACT_LOST);
			return true;
		}
	}

	/* If we lost an artifact that didn't previously have a history, then we
	 * missed it */
	history_add_artifact(p, artifact, false, false);

	return false;
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
