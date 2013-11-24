/*
 * File: z-msg.c
 * Purpose: Message handling
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
#include "z-term.h"
#include "z-util.h"
#include "z-msg.h"

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

/* Functions operating on the entire list */
errr messages_init(void)
{
	messages = ZNEW(msgqueue_t);
	messages->max = 2048;
	return 0;
}

void messages_free(void)
{
	msgcolor_t *c = messages->colors;
	msgcolor_t *nextc;
	message_t *m = messages->head;
	message_t *nextm;

	while (m)
	{
		nextm = m->older;
		FREE(m->str);
		FREE(m);
		m = nextm;
	}

	while (c)
	{
		nextc = c->next;
		FREE(c);
		c = nextc;
	}

	FREE(messages);
}

u16b messages_num(void)
{
	return messages->count;
}

/* Functions for individual messages */

void message_add(const char *str, u16b type)
{
	message_t *m;

	if (messages->head &&
	    messages->head->type == type &&
	    !strcmp(messages->head->str, str))
	{
		messages->head->count++;
		return;
	}

	m = ZNEW(message_t);
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

	if (messages->count > messages->max)
	{
		message_t *old_tail = messages->tail;

		messages->tail = old_tail->newer;
		messages->tail->older = NULL;
		FREE(old_tail->str);
		FREE(old_tail);
		messages->count--;
	}
}

static message_t *message_get(u16b age)
{
	message_t *m = messages->head;

	while (m && age--)
		m = m->older;

	return m;
}


const char *message_str(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->str : "");
}

u16b message_count(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->count : 0);
}

u16b message_type(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->type : 0);
}

byte message_color(u16b age)
{
	message_t *m = message_get(age);
	return (m ? message_type_color(m->type) : TERM_WHITE);
}


/* Message-color functions */

void message_color_define(u16b type, byte color)
{
	msgcolor_t *mc;

	if (!messages->colors)
	{
		messages->colors = ZNEW(msgcolor_t);
		messages->colors->type = type;
		messages->colors->color = color;
	}

	mc = messages->colors;
	while (mc->next)
	{
		if (mc->type == type)
		{
			mc->color = color;
		}
		mc = mc->next;
	}

	mc->next = ZNEW(msgcolor_t);
	mc->next->type = type;
	mc->next->color = color;
}

byte message_type_color(u16b type)
{
	msgcolor_t *mc;
	byte color = TERM_WHITE;

	if (messages)
	{
		mc = messages->colors;

		while (mc && mc->type != type)
			mc = mc->next;

		if (mc && (mc->color != TERM_DARK))
			color = mc->color;
	}

	return color;
}

int message_lookup_by_name(const char *name)
{
	static const char *message_names[] = {
		#define MSG(x, s) #x,
		#include "z-msg-list.h"
		#undef MSG
	};
	size_t i;
	unsigned int number;

	if (sscanf(name, "%u", &number) == 1)
		return (number < MSG_MAX) ? number : -1;

	for (i = 0; i < N_ELEMENTS(message_names); i++) {
		if (my_stricmp(name, message_names[i]) == 0)
			return (int)i;
	}

	return -1;
}

int message_lookup_by_sound_name(const char *name)
{
	static const char *sound_names[] = {
		#define MSG(x, s) s,
		#include "z-msg-list.h"
		#undef MSG
	};
	size_t i;

	for (i = 0; i < N_ELEMENTS(sound_names); i++) {
		if (my_stricmp(name, sound_names[i]) == 0)
			return (int)i;
	}

	return MSG_GENERIC;
}

const char *message_sound_name(int message)
{
	static const char *sound_names[] = {
		#define MSG(x, s) s,
		#include "z-msg-list.h"
		#undef MSG
	};

	if (message < MSG_GENERIC || message >= MSG_MAX)
		return NULL;

	return sound_names[message];
}
