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
#include "game-event.h"

struct event_handler_entry
{
	struct event_handler_entry *next;	
	game_event_handler *fn;
	void *user;
};

struct event_handler_entry *event_handlers[N_GAME_EVENTS];

static void game_event_dispatch(game_event_type type, game_event_data *data)
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

void event_add_handler(game_event_type type, game_event_handler *fn, void *user)
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

void event_remove_handler(game_event_type type, game_event_handler *fn, void *user)
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
}

void event_add_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user)
{
	int i;

	for (i = 0; i < n_types; i++)
	{
		event_add_handler(type[i], fn, user);
	}
}

void event_remove_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user)
{
	int i;

	for (i = 0; i < n_types; i++)
	{
		event_remove_handler(type[i], fn, user);
	}
}




void event_signal(game_event_type type)
{
	game_event_dispatch(type, NULL);
}


void event_signal_point(game_event_type type, int x, int y)
{
	game_event_data data;
	data.point.x = x;
	data.point.y = y;

	game_event_dispatch(type, &data);
}


void event_signal_string(game_event_type type, const char *s)
{
	game_event_data data;
	data.string = s;

	game_event_dispatch(type, &data);
}


void event_signal_birthstage_question(enum birth_stage stage, const char *hint, int n_choices, int initial_choice, const char *choices[], const char *helptexts[])
{
	game_event_data data;

	data.birthstage.stage = stage;
	data.birthstage.hint = hint;
	data.birthstage.n_choices = n_choices;
	data.birthstage.initial_choice = initial_choice;
	data.birthstage.choices = choices;
	data.birthstage.helptexts = helptexts;

	game_event_dispatch(EVENT_BIRTHSTAGE, &data);
}

void event_signal_birthstage(enum birth_stage stage, void *xtra)
{
	game_event_data data;

	data.birthstage.stage = stage;
	data.birthstage.xtra = xtra;

	game_event_dispatch(EVENT_BIRTHSTAGE, &data);
}

void event_signal_birthstats(int stats[6], int remaining)
{
	game_event_data data;

	data.birthstats.stats = stats;
	data.birthstats.remaining = remaining;

	game_event_dispatch(EVENT_BIRTHSTATS, &data);
}


void event_signal_birthautoroller(int limits[6], int matches[6], int current[6], unsigned long round)
{
	game_event_data data;

	data.birthautoroll.limits = limits;
	data.birthautoroll.matches = matches;
	data.birthautoroll.current = current;
	data.birthautoroll.round = round;

	game_event_dispatch(EVENT_BIRTHAUTOROLLER, &data);
}
