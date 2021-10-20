/**
 * \file message.c
 * \brief Message handling
 *
 * Copyright (c) 2007 Elly, Andi Sidwell
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */

#include "z-virt.h"
#include "z-color.h"
#include "z-util.h"
#include "message.h"
#include "game-event.h"
#include "option.h"
#include "init.h"
#include "player.h"

typedef struct _message_t
{
	char *str;
	struct _message_t *newer;
	struct _message_t *older;
	u16b type;
	u16b count;
} message_t;

typedef struct _msgcolor_t
{
	u16b type;
	byte color;
	struct _msgcolor_t *next;
} msgcolor_t;

typedef struct _msgqueue_t
{
	message_t *head;
	message_t *tail;
	msgcolor_t *colors;
	u32b count;
	u32b max;
} msgqueue_t;

static msgqueue_t *messages = NULL;

/**
 * ------------------------------------------------------------------------
 * Functions operating on the entire list
 * ------------------------------------------------------------------------ */
/**
 * Initialise the messages package.  Should be called before using any other
 * functions in the package.
 */
void messages_init(void)
{
	messages = mem_zalloc(sizeof(msgqueue_t));
	messages->max = 2048;
}

/**
 * Free the message package.
 */
void messages_free(void)
{
	msgcolor_t *c = messages->colors;
	msgcolor_t *nextc;
	message_t *m = messages->head;
	message_t *nextm;

	while (m) {
		nextm = m->older;
		mem_free(m->str);
		mem_free(m);
		m = nextm;
	}

	while (c) {
		nextc = c->next;
		mem_free(c);
		c = nextc;
	}

	mem_free(messages);
}

/**
 * Return the current number of messages stored.
 */
u16b messages_num(void)
{
	return messages->count;
}

/**
 * ------------------------------------------------------------------------
 * Functions for individual messages
 * ------------------------------------------------------------------------ */
/**
 * Save a new message into the memory buffer, with text `str` and type `type`.
 * The type should be one of the MSG_ constants defined in message.h.
 *
 * The new message may not be saved if it is identical to the one saved before
 * it, in which case the "count" of the message will be increased instead.
 * This count can be fetched using the message_count() function.
 */
void message_add(const char *str, u16b type)
{
	message_t *m;

	if (messages->head &&
	    messages->head->type == type &&
	    streq(messages->head->str, str) &&
	    messages->head->count != (u16b)-1) {
		messages->head->count++;
		return;
	}

	m = mem_zalloc(sizeof(message_t));
	m->str = string_make(str);
	m->type = type;
	m->count = 1;
	m->older = messages->head;

	if (messages->head)
		messages->head->newer = m;

	messages->head = m;
	messages->count++;

	if (!messages->tail)
		messages->tail = m;

	if (messages->count > messages->max) {
		message_t *old_tail = messages->tail;

		messages->tail = old_tail->newer;
		messages->tail->older = NULL;
		mem_free(old_tail->str);
		mem_free(old_tail);
		messages->count--;
	}
}

/**
 * Returns the message of age `age`.
 */
static message_t *message_get(u16b age)
{
	message_t *m = messages->head;

	while (m && age) {
		age--;
		m = m->older;
	}

	return m;
}


/**
 * Returns the text of the message of age `age`.  The age of the most recently
 * saved message is 0, the one before that is of age 1, etc.
 *
 * Returns the empty string if the no messages of the age specified are
 * available.
 */
const char *message_str(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->str : "");
}

/**
 * Returns the number of times the message of age `age` was saved. The age of
 * the most recently saved message is 0, the one before that is of age 1, etc.
 *
 * In other words, if message_add() was called five times, one after the other,
 * with the message "The orc sets your hair on fire.", then the text will only
 * have one age (age = 0), but will have a count of 5.
 */
u16b message_count(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->count : 0);
}

/**
 * Returns the type of the message of age `age`.  The age of the most recently
 * saved message is 0, the one before that is of age 1, etc.
 *
 * The type is one of the MSG_ constants, defined in message.h.
 */
u16b message_type(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->type : 0);
}

/**
 * Returns the display colour of the message memorised `age` messages ago.
 * (i.e. age = 0 represents the last memorised message, age = 1 is the one
 * before that, etc).
 */
byte message_color(u16b age)
{
	message_t *m = message_get(age);
	return (m ? message_type_color(m->type) : COLOUR_WHITE);
}


/**
 * ------------------------------------------------------------------------
 * Message-color functions
 * ------------------------------------------------------------------------ */
/**
 * Defines the color `color` for the message type `type`.
 */
void message_color_define(u16b type, byte color)
{
	msgcolor_t *mc;

	if (!messages->colors)
	{
		messages->colors = mem_zalloc(sizeof(msgcolor_t));
		messages->colors->type = type;
		messages->colors->color = color;
		return;
	}

	mc = messages->colors;
	while (1)
	{
		if (mc->type == type)
		{
			mc->color = color;
			break;
		}
		if (! mc->next) {
			mc->next = mem_zalloc(sizeof(msgcolor_t));
			mc->next->type = type;
			mc->next->color = color;
			break;
		}
		mc = mc->next;
	}
}

/**
 * Returns the colour for the message type `type`.
 */
byte message_type_color(u16b type)
{
	msgcolor_t *mc;
	byte color = COLOUR_WHITE;

	if (messages)
	{
		mc = messages->colors;

		while (mc && mc->type != type)
			mc = mc->next;

		if (mc && (mc->color != COLOUR_DARK))
			color = mc->color;
	}

	return color;
}

/**
 * Return the MSG_ flag that matches the given string. This does not handle
 * SOUND_MAX.
 *
 * \param name is a string that contains the name of a flag or a number.
 * \return The MSG_ flag that matches the given name.
 */
int message_lookup_by_name(const char *name)
{
	static const char *message_names[] = {
		#define MSG(x, s) #x,
		#include "list-message.h"
		#undef MSG
	};
	size_t i;
	unsigned int number;

	if (sscanf(name, "%u", &number) == 1)
		return (number < MSG_MAX) ? (int)number : -1;

	for (i = 0; i < N_ELEMENTS(message_names); i++) {
		if (my_stricmp(name, message_names[i]) == 0)
			return (int)i;
	}

	return -1;
}

/**
 * Return the MSG_ flag that matches the given sound event name.
 *
 * \param name is the sound name from sound.cfg.
 * \return The MSG_ flag for the corresponding sound.
 */
int message_lookup_by_sound_name(const char *name)
{
	static const char *sound_names[] = {
		#define MSG(x, s) s,
		#include "list-message.h"
		#undef MSG
	};
	size_t i;

	/* Exclude MSG_MAX since it has NULL for the sound's name. */
	for (i = 0; i < N_ELEMENTS(sound_names) - 1; i++) {
		if (my_stricmp(name, sound_names[i]) == 0)
			return (int)i;
	}

	return MSG_GENERIC;
}

/**
 * Return the sound name for the given message.
 *
 * \param message is the MSG_ flag to find.
 * \return The sound.cfg sound name.
 */
const char *message_sound_name(int message)
{
	static const char *sound_names[] = {
		#define MSG(x, s) s,
		#include "list-message.h"
		#undef MSG
	};

	if (message < MSG_GENERIC || message >= MSG_MAX)
		return NULL;

	return sound_names[message];
}

/**
 * Make a noise, without a message.  Sound modules hook into this event.
 * 
 * \param type MSG_* constant for the sound type
 */
void sound(int type)
{
	/* No sound */
	if (!OPT(player, use_sound)) return;

	/* Dispatch */
	event_signal_message(EVENT_SOUND, type, NULL);
}

/**
 * Ring the system bell.
 */
void bell(void)
{
	/* Send bell event */
	event_signal_message(EVENT_BELL, MSG_BELL, NULL);
}

/**
 * Display a formatted message.
 *
 * NB: Never call this function directly with a string read in from a
 * file, because it may contain format characters and crash the game.
 * Always use msg("%s", string) in those situations.
 *
 * \param fmt Format string
 */
void msg(const char *fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, sizeof(buf), fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Fail if messages not loaded */
	if (!messages) return;

	/* Add to message log */
	message_add(buf, MSG_GENERIC);

	/* Send refresh event */
	event_signal_message(EVENT_MESSAGE, MSG_GENERIC, buf);

}

/**
 * Display a formatted message with a given type, making a sound
 * relevant to the message tyoe.
 *
 * \param type MSG_ constant
 * \param fmt Format string
 */
void msgt(unsigned int type, const char *fmt, ...)
{
	va_list vp;
	char buf[1024];
	va_start(vp, fmt);
	vstrnfmt(buf, sizeof(buf), fmt, vp);
	va_end(vp);

	/* Fail if messages not loaded */
	if (!messages) return;

	/* Add to message log */
	message_add(buf, type);

	/* Send refresh event */
	sound(type);
	event_signal_message(EVENT_MESSAGE, type, buf);
}


struct init_module messages_module = {
	.name = "messages",
	.init = messages_init,
	.cleanup = messages_free
};
