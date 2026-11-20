/**
 * \file player-timed.h
 * \brief Timed effects handling
 *
 * Copyright (c) 1997 Ben Harrison
 * Copyright (c) 2007 A Sidwell <andi@takkaria.org>
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

#ifndef PLAYER_TIMED_H
#define PLAYER_TIMED_H

#include "player.h"

/**
 * Timed effects
 */
enum
{
	#define TMD(a, b, c) TMD_##a,
	#include "list-player-timed.h"
	#undef TMD
	TMD_MAX
};

/**
 * Effect failure flag types
 */
enum {
	TMD_FAIL_FLAG_OBJECT = 1,
	TMD_FAIL_FLAG_RESIST,
	TMD_FAIL_FLAG_VULN,
	TMD_FAIL_FLAG_PLAYER,
	TMD_FAIL_FLAG_TIMED_EFFECT
};

/**
 * Bits in timed_effect_data's flags field
 */
enum {
	/**
	 * A complete rest will only end normally if no timed effect with this
	 * flag affects the player
	 */
	TMD_FLAG_COMPLETE_REST = 0x01,
	/** Increases to duration will be blocked if effect is already active */
	TMD_FLAG_NONSTACKING = 0x02,
};

struct timed_grade {
	/** Zero-based index within the timed effect's list of grades */
	int grade;
	/**
	 * Color table index to use when displaying the grade's name on the
	 * status line
	 */
	uint8_t color;
	/** Maximum value for the timed effect to be in this grade or lower */
	int max;
	/**
	 * Name of the grade to show on the status line; if NULL, nothing
	 * for the timed effect is shown on the status line when in this grade
	 */
	char *name;
	/**
	 * Text of the message to be displayed when reaching this grade from
	 * a lower grade; may be NULL, in which case no message is displayed
	 */
	char *up_msg;
	/**
	 * Text of the message to be displayed when reaching this grade from
	 * a higher grade; may be NULL, in which case no message is displayed
	 */
	char *down_msg;
	/** The next grade, which has a larger maximum, in the linked list */
	struct timed_grade *next;
};

struct timed_failure {
	struct timed_failure *next;
	int code; /**< one of the TMD_FAIL_FLAG_* constants */
	int idx; /**< index for object or player flag, timed effect, element */
};

/**
 * Holds a timed effect's properties
 */
struct timed_effect_data {
	/**
	 * The effect's code name; one of the first arguments to TMD() in
	 * list-player-timed.h
	 */
	const char *name;
	/**
	 * The combination of PR_ constants specifying what needs to be redrawn
	 * when the effect changes grades or notification is explicitly
	 * requested; copied from the second argument to TMD() in
	 * list-player-timed.h
	 */
	uint32_t flag_redraw;
	/**
	 * The combination of PU_ constants specifying what needs to be
	 * recalculated when the effect changes grades or notification is
	 * explicitly requested; copied from the third argument to TMD() in
	 * list-player-timed.h
	 */
	uint32_t flag_update;

	/** Brief description of effect */
	char *desc;
	/**
	 * Text of message to issue when the effect lapses and notification
	 * has been requested; if NULL, a message is not issued in those
	 * conditions
	 */
	char *on_end;
	/**
	 * Text of message to issue when the duration of the effect increases
	 * and notification has been requested; if NULL, a message is not
	 * issued in those conditions
	 */
	char *on_increase;
	/**
	 * Text of message to issue when the effect decreases and notification
	 * has been requested; if NULL, a message is not issued in those
	 * conditions
	 */
	char *on_decrease;
	/**
	 * The message type (MSG_) to use for the on_increase, on_decrease,
	 * or grade change messages
	 */
	int msgt;
	/**
	 * Linked list of conditions that could cause a checked increase
	 * of the duration of the effect to fail; may be NULL if there are
	 * no such conditions
	 */
	struct timed_failure *fail;
	/**
	 * Linked list of the grades, in ascending order by the max field in
	 * the grade structure; never empty as each timed effect has an off
	 * grade with a maximum of zero and one or more grades with maximums
	 * greater than zero
	 */
	struct timed_grade *grade;
	/** This effect chain is triggered when the timed effect starts. */
	struct effect *on_begin_effect;
	/** This effect chain is triggered when the timed effect lapses. */
	struct effect *on_end_effect;
	/** Zero or a bitwise-or of one or more of the TMD_FLAG_ constants */
	bitflag flags;
	/** Enforced lower bound for the effect's duration; usually zero */
	int lower_bound;
	/**
	 * If not OF_NONE, it is the OF_ property this timed effect is (largely)
	 * synonymous to.
	 */
	int oflag_dup;
	/**
	 * If true, the timed effect is exactly the same, except for duration,
	 * as the object property specified by oflag_dup.  Otherwise, the timed
	 * effect has other effects which are not implied by that property.
	 */
	bool oflag_syn;
	/**
	 * If non-zero, temp_resist is the index of the element this timed
	 * effect grants a temporary resistance to.  If zero, the timed effect
	 * does not grant a temporary elemental resistance.
	 */
	int temp_resist;
	/**
	 * If non-zero, brands[temp_brand] is the brand this timed effect adds
	 * to equipped weapons.  If zero, the timed effect does not add a
	 * brand to equipped weapons.
	 */
	int temp_brand;
	/**
	 * If non-zero, slays[temp_slay] is the slay this timed effect adds
	 * to equipped weapons.  If zero, the timed effect does not add a
	 * slay to equipped weapons.
	 */
	int temp_slay;
};

/**
 * Holds state while parsing.  Exposed for use by unit test cases.
 */
struct timed_effect_parse_state {
	/** Points to timed effect being populated.  May be NULL. */
	struct timed_effect_data *t;
	/**
	 * Points to the most recent effect chain being modified in the timed
	 * effect.  May be NULL.
	 */
	struct effect *e;
};

/** Player food value; bloated */
extern int PY_FOOD_MAX;
/** Player food value; normal */
extern int PY_FOOD_FULL;
/** Player food value; hungry */
extern int PY_FOOD_HUNGRY;
/** Player food value; weak */
extern int PY_FOOD_WEAK;
/** Player food value; fainting */
extern int PY_FOOD_FAINT;
/** Player food value; starving */
extern int PY_FOOD_STARVE;

extern struct file_parser player_timed_parser;
extern struct timed_effect_data timed_effects[TMD_MAX];

int timed_name_to_idx(const char *name);
bool player_timed_grade_eq(struct player *p, int idx, const char *match);
bool player_set_timed(struct player *p, int idx, int v, bool notify,
	bool can_disturb);
bool player_inc_check(struct player *p, int idx, bool lore);
bool player_inc_timed(struct player *p, int idx, int v, bool notify,
	bool can_disturb, bool check);
bool player_dec_timed(struct player *p, int idx, int v, bool notify,
	bool can_disturb);
bool player_clear_timed(struct player *p, int idx, bool notify,
	bool can_disturb);
bool player_timed_continues_rest(const struct player *p);

#endif /* !PLAYER_TIMED_H */
