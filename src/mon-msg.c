/**
 * \file mon-msg.c
 * \brief Monster message code.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "mon-desc.h"
#include "mon-msg.h"
#include "mon-util.h"
#include "game-input.h"
#include "player-calcs.h"

/**
 * Maxinum number of stacked monster messages
 */
#define MAX_STORED_MON_MSG		200
#define MAX_STORED_MON_CODES	400

/**
 * Flags for whether monsters are offscreen or invisible
 */
#define MON_MSG_FLAG_OFFSCREEN	0x01
#define MON_MSG_FLAG_INVISIBLE	0x02

/**
 * Message tags
 *
 * Used to determine which order to display messages in
 */
enum delay_tag {
	MON_DELAY_TAG_DEFAULT = 0,
	MON_DELAY_TAG_DEATH,
};

/**
 * A stacked monster message entry
 */
struct monster_race_message {
	struct monster_race *race;	/* The race of the monster */
	int flags;					/* Flags */
	int msg_code;				/* The coded message */
	int count;					/* How many monsters triggered this message */
	bool delay;					/* Should this message be put off to the end */
	enum delay_tag tag;			/* To group delayed messages for better presentation */
};

/**
 * A (monster, message type) pair used for duplicate checking
 */
struct monster_message_history {
	struct monster *mon;	/* The monster */
	int message_code;		/* The coded message */
};

static int size_mon_hist = 0;
static int size_mon_msg = 0;
static struct monster_race_message mon_msg[MAX_STORED_MON_MSG];
static struct monster_message_history mon_message_hist[MAX_STORED_MON_CODES];

/**
 * An array of monster messages in order of monster message type.
 *
 * Singular and plural modifiers are encoded in the same string. Example:
 * "[is|are] hurt" is expanded to "is hurt" if you request the singular form.
 * The string is expanded to "are hurt" if the plural form is requested.
 *
 * The singular and plural parts are optional. Example:
 * "rear[s] up in anger" only includes a modifier for the singular form.
 *
 * Any of these strings can start with "~", in which case we consider that
 * string as a whole message, not as a part of a larger message. This
 * is useful to display Moria-like death messages.
 */
static const char *msg_repository[] = {
	#define MON_MSG(x, s) s,
	#include "list-mon-message.h"
	#undef MON_MSG
};

/**
 * Adds to the message queue a message describing a monster's reaction
 * to damage.
 */
void message_pain(struct monster *mon, int dam)
{
	int msg_code = MON_MSG_UNHARMED;

	/* Calculate damage levels */
	if (dam > 0) {
		/* Note -- subtle fix -CFT */
		long newhp = (long)(mon->hp);
		long oldhp = newhp + (long)(dam);
		long tmp = (newhp * 100L) / oldhp;
		int percentage = (int)(tmp);

		if (percentage > 95)		msg_code = MON_MSG_95;
		else if (percentage > 75)	msg_code = MON_MSG_75;
		else if (percentage > 50)	msg_code = MON_MSG_50;
		else if (percentage > 35)	msg_code = MON_MSG_35;
		else if (percentage > 20)	msg_code = MON_MSG_20;
		else if (percentage > 10)	msg_code = MON_MSG_10;
		else						msg_code = MON_MSG_0;
	}

	add_monster_message(mon, msg_code, false);
}

#define MSG_PARSE_NORMAL	0
#define MSG_PARSE_SINGLE	1
#define MSG_PARSE_PLURAL	2

/**
 * Returns a pointer to a statically allocatted string containing a formatted
 * message based on the given message code and the quantity flag.
 *
 * The contents of the returned value will change with the next call
 * to this function.
 */
static char *get_mon_msg_action(int msg_code, bool do_plural,
								const struct monster_race *race)
{
	assert(msg_code < MON_MSG_MAX);
	assert(race != NULL);
	assert(race->base != NULL);
	assert(race->base->pain != NULL);

	/* Find the appropriate message */
	const char *source = msg_repository[msg_code];
	switch (msg_code) {
		case MON_MSG_95: source = race->base->pain->messages[0]; break;
		case MON_MSG_75: source = race->base->pain->messages[1]; break;
		case MON_MSG_50: source = race->base->pain->messages[2]; break;
		case MON_MSG_35: source = race->base->pain->messages[3]; break;
		case MON_MSG_20: source = race->base->pain->messages[4]; break;
		case MON_MSG_10: source = race->base->pain->messages[5]; break;
		case MON_MSG_0:  source = race->base->pain->messages[6]; break;
	}

	static char buf[200];
	size_t maxlen = MIN(strlen(source), sizeof(buf));
	size_t i;

	int state = MSG_PARSE_NORMAL;

	/* Put the message characters in the buffer */
	/* XXX This logic should be used everywhere for pluralising strings */
	for (i = 0; i < maxlen; i++) {
		char cur = source[i];

		/*
		 * The characters '[|]' switch parsing mode and are never output.
		 * The syntax is [singular|plural]
		 */
		if (state == MSG_PARSE_NORMAL        && cur == '[') {
			state = MSG_PARSE_SINGLE;
		} else if (state == MSG_PARSE_SINGLE && cur == '|') {
			state = MSG_PARSE_PLURAL;
		} else if (state != MSG_PARSE_NORMAL && cur == ']') {
			state = MSG_PARSE_NORMAL;
		} else {
			/* If we're parsing then we do these things */
			if (state == MSG_PARSE_NORMAL ||
					(state == MSG_PARSE_SINGLE && do_plural == false) ||
					(state == MSG_PARSE_PLURAL && do_plural == true)) {
				buf[i] = cur;
			}
		}
	}

	/* We should always return to the normal state */
	assert(state == MSG_PARSE_NORMAL);

	/* Terminate the buffer */
	buf[i] = '\0';

	/* Done */
	return buf;
}

#undef MSG_PARSE_NORMAL
#undef MSG_PARSE_SINGLE
#undef MSG_PARSE_PLURAL

/**
 * Tracks which monster has had which pain message stored, so redundant
 * messages don't happen due to monster attacks hitting other monsters.
 * Returns true if the message is redundant.
 */
static bool redundant_monster_message(struct monster *mon, int msg_code)
{
	assert(mon);
	assert(msg_code >= 0);
	assert(msg_code < MON_MSG_MAX);

	for (int i = 0; i < size_mon_hist; i++) {
		/* Check for a matched monster & monster code */
		if (mon == mon_message_hist[i].mon &&
				msg_code != mon_message_hist[i].message_code) {
			return true;
		}
	}

	return false;
}

/**
 * Work out what flags a message should have from a monster
 */
static int message_flags(const struct monster *mon)
{
	int flags = 0;

	if (!panel_contains(mon->fy, mon->fx)) {
		flags |= MON_MSG_FLAG_OFFSCREEN;
	}

	if (!mflag_has(mon->mflag, MFLAG_VISIBLE)) {
		flags |= MON_MSG_FLAG_INVISIBLE;
	}

	return flags;
}

/**
 * Store the monster in the monster history for duplicate checking later
 */
static void store_monster(struct monster *mon, int msg_code)
{
	/* Record which monster had this message stored */
	if (size_mon_hist < MAX_STORED_MON_CODES) {
		mon_message_hist[size_mon_hist].mon = mon;
		mon_message_hist[size_mon_hist].message_code = msg_code;
		size_mon_hist++;
	}
}

/**
 * Try to stack a message on top of existing ones
 *
 * \returns true if successful, false if failed
 */
static bool stack_message(struct monster *mon, int msg_code, int flags)
{
	int i;

	for (i = 0; i < size_mon_msg; i++) {
		/* We found the race and the message code */
		if (mon_msg[i].race == mon->race &&
					mon_msg[i].flags == flags &&
					mon_msg[i].msg_code == msg_code) {
			mon_msg[i].count++;
			store_monster(mon, msg_code);
			return true;
		}
	}

	return false;
}

/**
 * Stack a codified message for the given monster race. You must supply
 * the description of some monster of this race. You can also supply
 * different monster descriptions for the same race.
 * Return true on success.
 */
bool add_monster_message(struct monster *mon, int msg_code, bool delay)
{
	assert(msg_code >= 0);
	assert(msg_code < MON_MSG_MAX);

	if (redundant_monster_message(mon, msg_code))
		return false;

	int flags = message_flags(mon);

	/* Stack message on top of older message if possible */
	/* If not possible, check we have storage space for more messages and add */
	if (!stack_message(mon, msg_code, flags) &&
			size_mon_msg < MAX_STORED_MON_MSG) {
		int idx = size_mon_msg;

		/* Assign the message data to the free slot */
		mon_msg[idx].race = mon->race;
		mon_msg[idx].flags = flags;
		mon_msg[idx].msg_code = msg_code;
		mon_msg[idx].count = 1;

		/* Force all death messages to go at the end of the group for
		 * logical presentation */
		if (msg_code == MON_MSG_DIE || msg_code == MON_MSG_DESTROYED) {
			mon_msg[idx].delay = true;
			mon_msg[idx].tag = MON_DELAY_TAG_DEATH;
		} else {
			mon_msg[idx].delay = delay;
			mon_msg[idx].tag = MON_DELAY_TAG_DEFAULT;
		}

		size_mon_msg++;

		store_monster(mon, msg_code);

		player->upkeep->notice |= PN_MON_MESSAGE;

		return true;
	} else {
		return false;
	}
}

/**
 * Show and delete the stacked monster messages.
 *
 * Some messages are delayed so that they show up after everything else,
 * so we only display messages matching the delay parameter. This is to
 * avoid things like "The snaga dies. The snaga runs in fear!"
 */
static void show_monster_messages(bool delay, enum delay_tag tag)
{
	const struct monster_race *race;
	int i, count;
	char buf[512];
	char *action;
	bool action_only;

	/* Show every message */
	for (i = 0; i < size_mon_msg; i++) {
		int type = MSG_GENERIC;

		if (mon_msg[i].delay != delay) continue;

		/* Skip if we are delaying and the tags don't match */
		if (mon_msg[i].delay && mon_msg[i].tag != tag) continue;

		/* Cache the monster count */
		count = mon_msg[i].count;

		/* Paranoia */
		if (count < 1) continue;

		/* Start with an empty string */
		buf[0] = '\0';

		/* Cache the race index */
		race = mon_msg[i].race;

		/* Get the proper message action */
		action = get_mon_msg_action(mon_msg[i].msg_code, (count > 1), race);

		/* Monster is marked as invisible */
		if (mon_msg[i].flags & MON_MSG_FLAG_INVISIBLE) race = NULL;

		/* Special message? */
		action_only = (*action == '~');

		/* Format the proper message depending on type, number and visibility */
		if (race && !action_only) {
			char race_name[80];

			/* Get the race name */
			my_strcpy(race_name, race->name, sizeof(race_name));

			/* Uniques, multiple monsters, or just one */
			if (rf_has(race->flags, RF_UNIQUE)) {
				/* Just copy the race name */
				my_strcpy(buf, (race->name), sizeof(buf));
			} else if (count > 1) {
				/* Get the plural of the race name */
				if (race->plural != NULL) {
					my_strcpy(race_name, race->plural, sizeof(race_name));
				} else {
					plural_aux(race_name, sizeof(race_name));
				}

				/* Put the count and the race name together */
				strnfmt(buf, sizeof(buf), "%d %s", count, race_name);
			} else {
				/* Just add a slight flavor */
				strnfmt(buf, sizeof(buf), "the %s", race_name);
			}
		} else if (!race && !action_only) {
			if (count > 1) {
				/* Show the counter */
				strnfmt(buf, sizeof(buf), "%d monsters", count);
			} else {
				/* Just one non-visible monster */
				my_strcpy(buf, "it", sizeof(buf));
			}
		}

		/* Special message. Nuke the mark */
		if (action_only)
			++action;
		/* Regular message */
		else {
			/* Add special mark. Monster is offscreen */
			if (mon_msg[i].flags & MON_MSG_FLAG_OFFSCREEN)
				my_strcat(buf, " (offscreen)", sizeof(buf));

			/* Add the separator */
			my_strcat(buf, " ", sizeof(buf));
		}

		/* Append the action to the message */
		my_strcat(buf, action, sizeof(buf));

		/* Capitalize the message */
		*buf = toupper((unsigned char)*buf);

		switch (mon_msg[i].msg_code) {
			case MON_MSG_FLEE_IN_TERROR:
				type = MSG_FLEE;
				break;

			case MON_MSG_MORIA_DEATH:
			case MON_MSG_DESTROYED:
			case MON_MSG_DIE:
			case MON_MSG_SHRIVEL_LIGHT:
			case MON_MSG_DISENTEGRATES:
			case MON_MSG_FREEZE_SHATTER:
			case MON_MSG_DISSOLVE:
			{
				/* Assume normal death sound */
				type = MSG_KILL;

				/* Play a special sound if the monster was unique */
				if (race != NULL && rf_has(race->flags, RF_UNIQUE)) {
					if (race->base == lookup_monster_base("Morgoth"))
						type = MSG_KILL_KING;
					else
						type = MSG_KILL_UNIQUE;
				}
				break;
			}

		}

		/* Show the message */
		msgt(type, "%s", buf);
   }
}

/**
 * Print and delete all stacked monster messages.
 */
void flush_all_monster_messages(void)
{
	/* Flush regular messages, then delayed messages */
	show_monster_messages(false, MON_DELAY_TAG_DEFAULT);
	show_monster_messages(true, MON_DELAY_TAG_DEFAULT);
	show_monster_messages(true, MON_DELAY_TAG_DEATH);

	/* Delete all the stacked messages and history */
	size_mon_msg = 0;
	size_mon_hist = 0;
}
