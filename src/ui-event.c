/*
 * File: ui-event.c
 * Purpose: Allows the registering of handlers to be told about ui "events",
 *          and the game to signal these events to the UI.
 *
 * Copyright (c) 2007 Antony Sidwell
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


#include <assert.h>
#include "z-virt.h"
#include "ui-event.h"

struct event_handler_entry;
struct event_handler_entry
{
	struct event_handler_entry *next;	
	ui_event_handler *fn;
	void *user;
};

struct event_handler_entry *event_handlers[N_UI_EVENTS];

static void ui_event_dispatch(ui_event_type type, ui_event_data *data)
{
	struct event_handler_entry *this = event_handlers[type];

	/* 
	 * Send the word out to all interested event handlers.
	 */
	while (this)
	{
		/* Call the handler with the relevant data */
		this->fn(type, data, this->user);
		this = this->next;
	}
}

void ui_event_register(ui_event_type type, ui_event_handler *fn, void *user)
{
	struct event_handler_entry *new;

	assert(fn != NULL);

	/* Make a new entry */
	new = mem_alloc(sizeof *new);
	new->fn = fn;
	new->user = user;

	/* Add it to the head of the appropriate list */
	new->next = event_handlers[type];
	event_handlers[type] = new;
}

void ui_event_deregister(ui_event_type type, ui_event_handler *fn, void *user)
{
	struct event_handler_entry *prev = NULL;
	struct event_handler_entry *this = event_handlers[type];

	/* Look for the entry in the list */
	while (this)
	{
		/* Check if this is the entry we want to remove */
		if (this->fn == fn && this->user == user)
		{
			if (!prev)
			{
				event_handlers[type] = this->next;
			}
			else
			{
				prev->next = this->next;
			}

			mem_free(this);
			return;
		}

		prev = this;
		this = this->next;
	}
};

void ui_event_register_set(ui_event_type *type, size_t n_types, ui_event_handler *fn, void *user)
{
	int i;

	for (i = 0; i < n_types; i++)
	{
		ui_event_register(type[i], fn, user);
	}
}

void ui_event_deregister_set(ui_event_type *type, size_t n_types, ui_event_handler *fn, void *user)
{
	int i;

	for (i = 0; i < n_types; i++)
	{
		ui_event_deregister(type[i], fn, user);
	}
}




void ui_event_signal(ui_event_type type)
{
	ui_event_dispatch(type, NULL);
}

void ui_event_signal_point(ui_event_type type, int x, int y)
{
	ui_event_data data;
	data.point.x = x;
	data.point.y = y;

	ui_event_dispatch(type, &data);
}
