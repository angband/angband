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
#include "init.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-msg.h"
#include "mon-util.h"
#include "player-calcs.h"

static u16b size_mon_hist = 0;
static u16b size_mon_msg = 0;

monster_race_message *mon_msg;
monster_message_history *mon_message_hist;

/**
 * The NULL-terminated array of string actions used to format stacked messages.
 * Singular and plural modifiers are encoded in the same string. Example:
 * "[is|are] hurt" is expanded to "is hurt" if you request the singular form.
 * The string is expanded to "are hurt" if the plural form is requested.
 * The singular and plural parts are optional. Example:
 * "rear[s] up in anger" only includes a modifier for the singular form.
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
	long oldhp, newhp, tmp;
	int percentage;

	int msg_code = MON_MSG_UNHARMED;
	char m_name[80];

	/* Get the monster name  - don't use monster_desc flags because
	 * add_monster_message does string processing on m_name */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_DEFAULT);

	/* Notice non-damage */
	if (dam == 0) {
		add_monster_message(m_name, mon, msg_code, FALSE);
		return;
	}

	/* Note -- subtle fix -CFT */
	newhp = (long)(mon->hp);
	oldhp = newhp + (long)(dam);
	tmp = (newhp * 100L) / oldhp;
	percentage = (int)(tmp);

	if (percentage > 95)
		msg_code = MON_MSG_95;
	else if (percentage > 75)
		msg_code = MON_MSG_75;
	else if (percentage > 50)
		msg_code = MON_MSG_50;
	else if (percentage > 35)
		msg_code = MON_MSG_35;
	else if (percentage > 20)
		msg_code = MON_MSG_20;
	else if (percentage > 10)
		msg_code = MON_MSG_10;
	else
		msg_code = MON_MSG_0;

	add_monster_message(m_name, mon, msg_code, FALSE);
}

#define SINGULAR_MON   1
#define PLURAL_MON     2
           
/**
 * Returns a pointer to a statically allocatted string containing a formatted
 * message based on the given message code and the quantity flag.
 * The contents of the returned value will change with the next call
 * to this function
 */
static char *get_mon_msg_action(byte msg_code, bool do_plural,
								const struct monster_race *race)
{
	static char buf[200];
	const char *action;
	u16b n = 0;

	/* Regular text */
	byte flag = 0;

	assert(msg_code < MON_MSG_MAX);
	action = msg_repository[msg_code];
	
	assert(race->base && race->base->pain);

	if (race->base && race->base->pain) {
		switch (msg_code) {
			case MON_MSG_95: action = race->base->pain->messages[0];
				break;
			case MON_MSG_75: action = race->base->pain->messages[1];
				break;
			case MON_MSG_50: action = race->base->pain->messages[2];
				break;
			case MON_MSG_35: action = race->base->pain->messages[3];
				break;
			case MON_MSG_20: action = race->base->pain->messages[4];
				break;
			case MON_MSG_10: action = race->base->pain->messages[5];
				break;
			case MON_MSG_0: action = race->base->pain->messages[6];
				break;
		}
	}

	/* Put the message characters in the buffer */
	for (; *action; action++) {
		/* Check available space */
		if (n >= (sizeof(buf) - 1)) break;

		/* Are we parsing a quantity modifier? */
		if (flag) {
			/* Check the presence of the modifier's terminator */
			if (*action == ']') {
				/* Go back to parsing regular text */
				flag = 0;

				/* Skip the mark */
				continue;
			}

			/* Check if we have to parse the plural modifier */
			if (*action == '|') {
				/* Switch to plural modifier */
				flag = PLURAL_MON;

				/* Skip the mark */
				continue;
			}

			/* Ignore the character if we need the other part */
			if ((flag == PLURAL_MON) != do_plural) continue;
		} else if (*action == '[') {
			/* Switch to singular modifier */
			flag = SINGULAR_MON;

			/* Skip the mark */
			continue;
		}

		/* Append the character to the buffer */
		buf[n++] = *action;
	}

	/* Terminate the buffer */
	buf[n] = '\0';

	/* Done */
	return (buf);
}

/**
 * Tracks which monster has had which pain message stored, so redundant
 * messages don't happen due to monster attacks hitting other monsters.
 * Returns TRUE if the message is redundant.
 */
static bool redundant_monster_message(struct monster *mon, int msg_code)
{
	int i;

	assert(mon);
	assert(msg_code >= 0 && msg_code < MON_MSG_MAX);

	/* No messages yet */
	if (!size_mon_hist) return FALSE;

	for (i = 0; i < size_mon_hist; i++) {
		/* Not the same monster */
		if (mon != mon_message_hist[i].mon) continue;

		/* Not the same code */
		if (msg_code != mon_message_hist[i].message_code) continue;

		/* We have a match. */
		return (TRUE);
	}

	return (FALSE);
}


/**
 * Stack a codified message for the given monster race. You must supply
 * the description of some monster of this race. You can also supply
 * different monster descriptions for the same race.
 * Return TRUE on success.
 */
bool add_monster_message(const char *mon_name, struct monster *mon,
		int msg_code, bool delay)
{
	int i;
	byte mon_flags = 0;

	assert(msg_code >= 0 && msg_code < MON_MSG_MAX);

	if (redundant_monster_message(mon, msg_code)) return (FALSE);

	/* Paranoia */
	if (!mon_name || !mon_name[0]) mon_name = "it";

	/* Save the "hidden" mark, if present */
	if (strstr(mon_name, "(hidden)")) mon_flags |= MON_MSG_FLAG_HIDDEN;

	/* Save the "offscreen" mark, if present */
	if (strstr(mon_name, "(offscreen)")) mon_flags |= MON_MSG_FLAG_OFFSCREEN;

	/* Monster is invisible or out of LOS */
	if (streq(mon_name, "it") || streq(mon_name, "something"))
		mon_flags |= MON_MSG_FLAG_INVISIBLE;

	/* Query if the message is already stored */
	for (i = 0; i < size_mon_msg; i++) {
		/* We found the race and the message code */
		if ((mon_msg[i].race == mon->race) &&
			(mon_msg[i].mon_flags == mon_flags) &&
			(mon_msg[i].msg_code == msg_code)) {
			/* Can we increment the counter? */
			if (mon_msg[i].mon_count < MAX_UCHAR) {
				/* Stack the message */
				++(mon_msg[i].mon_count);
			}
   
			/* Success */
			return (TRUE);
		}
	}
   
	/* The message isn't stored. Check free space */
	if (size_mon_msg >= MAX_STORED_MON_MSG) return (FALSE);

	/* Assign the message data to the free slot */
	mon_msg[i].race = mon->race;
	mon_msg[i].mon_flags = mon_flags;
	mon_msg[i].msg_code = msg_code;
	mon_msg[i].delay = delay;
	mon_msg[i].delay_tag = MON_DELAY_TAG_DEFAULT;
	/* Just this monster so far */
	mon_msg[i].mon_count = 1;

	/* Force all death messages to go at the end of the group for
	 * logical presentation */
	if (msg_code == MON_MSG_DIE || msg_code == MON_MSG_DESTROYED) {
		mon_msg[i].delay = TRUE;
		mon_msg[i].delay_tag = MON_DELAY_TAG_DEATH;
	}

	/* One more entry */
	++size_mon_msg;
 
	player->upkeep->notice |= PN_MON_MESSAGE;

	/* record which monster had this message stored */
	if (size_mon_hist >= MAX_STORED_MON_CODES) return (TRUE);
	mon_message_hist[size_mon_hist].mon = mon;
	mon_message_hist[size_mon_hist].message_code = msg_code;
	size_mon_hist++;

	/* Success */
	return (TRUE);
}

/**
 * Show and delete the stacked monster messages.
 * Some messages are delayed so that they show up after everything else.
 * This is to avoid things like "The snaga dies. The snaga runs in fear!"
 * So we only flush messages matching the delay parameter.
 */
static void flush_monster_messages(bool delay, byte delay_tag)
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
		if (mon_msg[i].delay && mon_msg[i].delay_tag != delay_tag) continue;
   
		/* Cache the monster count */
		count = mon_msg[i].mon_count;

		/* Paranoia */
		if (count < 1) continue;

		/* Start with an empty string */
		buf[0] = '\0';

		/* Cache the race index */
		race = mon_msg[i].race;
		   
		/* Get the proper message action */
		action = get_mon_msg_action(mon_msg[i].msg_code, (count > 1), race);

		/* Monster is marked as invisible */
		if (mon_msg[i].mon_flags & MON_MSG_FLAG_INVISIBLE) race = NULL;

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
			if (mon_msg[i].mon_flags & MON_MSG_FLAG_OFFSCREEN)
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
	flush_monster_messages(FALSE, MON_DELAY_TAG_DEFAULT);
	flush_monster_messages(TRUE, MON_DELAY_TAG_DEFAULT);
	flush_monster_messages(TRUE, MON_DELAY_TAG_DEATH);

	/* Delete all the stacked messages and history */
	size_mon_msg = 0;
	size_mon_hist = 0;
}

static void monmsg_init(void) {
	/* Array of stacked monster messages */
	mon_msg = mem_zalloc(MAX_STORED_MON_MSG * sizeof(monster_race_message));
	mon_message_hist = mem_zalloc(MAX_STORED_MON_CODES *
								  sizeof(monster_message_history));
}

static void monmsg_cleanup(void) {
	/* Free the stacked monster messages */
	mem_free(mon_msg);
	mem_free(mon_message_hist);
}

struct init_module monmsg_module = {
	.name = "mosnter message",
	.init = monmsg_init,
	.cleanup = monmsg_cleanup
};
