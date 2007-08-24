/*
 * File: ui-event.c
 * Purpose: Generic input event handling
 *
 * Copyright (c) 2007 Pete Mack and others.
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

/*
 * Implementation of Extremely Basic Event Model.
 * Limits:
 *   all events are of the concrete type ui_event_type (see ui-event.h), 
 *   which are supposed to model simple UI actions:
 *	- < escape >
 *	- keystroke
 *	- mousepress
 *	- select menu element
 *	- move menu cursor
 *	- back to parent (hierarchical menu escape)
 *
 * There are 3 basic event-related classes:
 * The ui_event_type.
 * Concrete event, with at most 32 distinct types.
 *
 * The event_listener observer for key events
 *
 * The event_target   The registrar for event_listeners.
 * For convenience, the event target is also an event_listener.
 */

/* List of event listeners--Helper class for event_target and the event loop */
struct listener_list
{
	event_listener *listener;
	struct listener_list *next;
};


void stop_event_loop()
{
	ui_event_data stop = { EVT_STOP, 0, 0, 0, 0 };

	/* Stop right away! */
	Term_event_push(&stop);
}

/*
 * Primitive event loop.
 *
 *  - target = the event target
 *  - forever - if false, stop at first unhandled event. Otherwise, stop only
 *    for STOP events
 *  - start - optional initial event that allows you to prime the loop without
 *    pushing the event queue.
 *  Returns:
 *    EVT_STOP - the loop was halted.
 *    EVT_AGAIN - start was not handled, and forever is false
 *    The first unhandled event - forever is false.
 */
ui_event_data run_event_loop(event_target *target, bool forever, const ui_event_data *start)
{
	ui_event_data ke = EVENT_EMPTY;
	bool handled = TRUE;

	while (forever || handled)
	{
		listener_list *list = target->observers;
		handled = FALSE;

		if (start) ke = *start;
		else ke = inkey_ex();

		if (ke.type == EVT_STOP)
			break;

		if (ke.type & target->self.events.evt_flags)
			handled = target->self.handler(target->self.object, &ke);

		if (!target->is_modal)
		{
			while (list && !handled)
			{
				if (ke.type & list->listener->events.evt_flags)
					handled = list->listener->handler(list->listener->object, &ke);

				list = list->next;
			}
		}

		if (handled) start = NULL;
	}

	if (start)
	{
		ke.type = EVT_AGAIN;
		ke.key = '\xff';
	}

	return ke;
}

void add_listener(event_target *target, event_listener *observer)
{
	listener_list *link;

	link = ZNEW(listener_list);
	link->listener = observer;
	link->next = target->observers;
	target->observers = link;
}

void remove_listener(event_target *target, event_listener *observer)
{
	listener_list *cur = target->observers;
	listener_list **prev = &target->observers;

	while (cur)
	{
		if (cur->listener == observer)
		{
			*prev = cur->next;
			FREE(cur);
			break;
		}
	}

	bell("remove_listener: no such observer");
}


