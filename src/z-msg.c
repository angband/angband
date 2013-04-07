/*
 * File: z-msg.c
 * Purpose: Message handling
 *
 * Copyright (c) 2007 Elly, Andrew Sidwell
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
#include "z-msg.h"

typedef struct _message_t
{
	char *str;
	struct _message_t *newer;
	struct _message_t *older;
	uint16_t type;
	uint16_t count;
} message_t;

typedef struct _msgcolor_t
{
	uint16_t type;
	uint8_t color;
	struct _msgcolor_t *next;
} msgcolor_t;

typedef struct _msgqueue_t
{
	message_t *head;
	message_t *tail;
	msgcolor_t *colors;
	uint32_t count;
	uint32_t max;
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

uint16_t messages_num(void)
{
	return messages->count;
}

/* Functions for individual messages */

void message_add(const char *str, uint16_t type)
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

static message_t *message_get(uint16_t age)
{
	message_t *m = messages->head;

	while (m && age--)
		m = m->older;

	return m;
}


const char *message_str(uint16_t age)
{
	message_t *m = message_get(age);
	return (m ? m->str : "");
}

uint16_t message_count(uint16_t age)
{
	message_t *m = message_get(age);
	return (m ? m->count : 0);
}

uint16_t message_type(uint16_t age)
{
	message_t *m = message_get(age);
	return (m ? m->type : 0);
}

uint8_t message_color(uint16_t age)
{
	message_t *m = message_get(age);
	return (m ? message_type_color(m->type) : TERM_WHITE);
}


/* Message-color functions */

void message_color_define(uint16_t type, uint8_t color)
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

uint8_t message_type_color(uint16_t type)
{
	msgcolor_t *mc;
	uint8_t color = TERM_WHITE;

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
