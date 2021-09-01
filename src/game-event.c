/**
 * \file game-event.c
 * \brief Allows the registering of handlers to be told about game events.
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
#include "game-event.h"
#include "object.h"
#include "z-virt.h"

struct event_handler_entry
{
	struct event_handler_entry *next;	
	game_event_handler *fn;
	void *user;
};

static struct event_handler_entry *event_handlers[N_GAME_EVENTS];

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

void event_remove_handler_type(game_event_type type)
{
	struct event_handler_entry *handler = event_handlers[type];

	while (handler) {
		struct event_handler_entry *next = handler->next;
		mem_free(handler);
		handler = next;
	}
	event_handlers[type] = NULL;
}

void event_remove_all_handlers(void)
{
	int type;
	struct event_handler_entry *handler, *next;

	for (type = 0; type < N_GAME_EVENTS; type++) {
		handler = event_handlers[type];
		while (handler) {
			next = handler->next;
			mem_free(handler);
			handler = next;
		}
		event_handlers[type] = NULL;
	}
}

void event_add_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user)
{
	size_t i;

	for (i = 0; i < n_types; i++)
		event_add_handler(type[i], fn, user);
}

void event_remove_handler_set(game_event_type *type, size_t n_types, game_event_handler *fn, void *user)
{
	size_t i;

	for (i = 0; i < n_types; i++)
		event_remove_handler(type[i], fn, user);
}




void event_signal(game_event_type type)
{
	game_event_dispatch(type, NULL);
}

void event_signal_flag(game_event_type type, bool flag)
{
	game_event_data data;
	data.flag = flag;

	game_event_dispatch(type, &data);
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

void event_signal_message(game_event_type type, int t, const char *s)
{
	game_event_data data;
	memset(&data, 0, sizeof data);

	data.message.type = t;
	data.message.msg = s;

	game_event_dispatch(type, &data);
}

/**
 * Signal a change or refresh in the point buy for birth stats.
 *
 * \param points points[i] is the number of points already spent to increase
 * the ith stat, i >= 0 and i < STAT_MAX.
 * \param inc_points inc_points[i] is the number of additional points it would
 * take to incrase the ith stat by one, i >= 0 and i < STAT_MAX.
 * \param remaining is the number of poitns that remain to be spent.
 */
void event_signal_birthpoints(const int *points, const int *inc_points,
		int remaining)
{
	game_event_data data;

	data.birthpoints.points = points;
	data.birthpoints.inc_points = inc_points;
	data.birthpoints.remaining = remaining;

	game_event_dispatch(EVENT_BIRTHPOINTS, &data);
}

void event_signal_blast(game_event_type type,
						int proj_type,
						int num_grids,
						int *distance_to_grid,
						bool drawing,
						bool *player_sees_grid,
						struct loc *blast_grid,
						struct loc centre)
{
	game_event_data data;
	data.explosion.proj_type = proj_type;
	data.explosion.num_grids = num_grids;
	data.explosion.distance_to_grid = distance_to_grid;
	data.explosion.drawing = drawing;
	data.explosion.player_sees_grid = player_sees_grid;
	data.explosion.blast_grid = blast_grid;
	data.explosion.centre = centre;

	game_event_dispatch(type, &data);
}

void event_signal_bolt(game_event_type type,
					   int proj_type,
					   bool drawing,
					   bool seen,
					   bool beam,
					   int oy,
					   int ox,
					   int y,
					   int x)
{
	game_event_data data;
	data.bolt.proj_type = proj_type;
	data.bolt.drawing = drawing;
	data.bolt.seen = seen;
	data.bolt.beam = beam;
	data.bolt.oy = oy;
	data.bolt.ox = ox;
	data.bolt.y = y;
	data.bolt.x = x;

	game_event_dispatch(type, &data);
}

void event_signal_missile(game_event_type type,
						  struct object *obj,
						  bool seen,
						  int y,
						  int x)
{
	game_event_data data;
	data.missile.obj = obj;
	data.missile.seen = seen;
	data.missile.y = y;
	data.missile.x = x;

	game_event_dispatch(type, &data);
}

void event_signal_size(game_event_type type, int h, int w)
{
	game_event_data data;

	data.size.h = h;
	data.size.w = w;
	game_event_dispatch(type, &data);
}

void event_signal_tunnel(game_event_type type, int nstep, int npierce, int ndug,
		int dstart, int dend, bool early)
{
	game_event_data data;

	data.tunnel.nstep = nstep;
	data.tunnel.npierce = npierce;
	data.tunnel.ndug = ndug;
	data.tunnel.dstart = dstart;
	data.tunnel.dend = dend;
	data.tunnel.early = early;
	game_event_dispatch(type, &data);
}
