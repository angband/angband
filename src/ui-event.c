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
 * The event_listener   The registrar for event_listeners.
 * For convenience, the event target is also an event_listener.
 */




/*
 * Primitive event loop.
 *
 * handler: function to call for each event
 * data: data to give to that function
 * event_flags: events that the function is interested in
 * start: optional initial event that allows you to prime the loop without pushing the event queue
 *
 *  Returns the first unhandled event.
 */
ui_event_data run_event_loop(bool (*handler)(void *object, const ui_event_data *in), void *data, int event_flags, const ui_event_data *start)
{
	ui_event_data ke;
	bool handled = TRUE;

	if (start)
	{
		ke = *start;
		if (ke.type & event_flags)
			handled = handler(data, &ke);
	}

	while (handled)
	{
		ke = inkey_ex();

		if (ke.type & event_flags)
			handled = handler(data, &ke);
		else
			break;
	}

	return ke;
}
